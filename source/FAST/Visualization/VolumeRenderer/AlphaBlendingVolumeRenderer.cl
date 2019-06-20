const sampler_t volumeSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

// intersect ray with a box
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
int intersectBox(float4 r_o, float4 r_d, float4 boxmin, float4 boxmax, float *tnear, float *tfar) {
    // compute intersection of ray with all six bbox planes
    float4 invR = (float4)(1.0f,1.0f,1.0f,1.0f) / r_d;
    float4 tbot = invR * (boxmin - r_o);
    float4 ttop = invR * (boxmax - r_o);

    // re-order intersections to find smallest and largest on each axis
    float4 tmin = min(ttop, tbot);
    float4 tmax = max(ttop, tbot);

    // find the largest tmin and the smallest tmax
    float largest_tmin = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    float smallest_tmax = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));

    *tnear = largest_tmin;
    *tfar = smallest_tmax;

    return smallest_tmax > largest_tmin;
}

uint rgbaFloatToInt(float4 rgba)
{
    rgba.x = clamp(rgba.x,0.0f,1.0f);
    rgba.y = clamp(rgba.y,0.0f,1.0f);
    rgba.z = clamp(rgba.z,0.0f,1.0f);
    rgba.w = clamp(rgba.w,0.0f,1.0f);
    return ((uint)(rgba.w*255.0f)<<24) | ((uint)(rgba.z*255.0f)<<16) | ((uint)(rgba.y*255.0f)<<8) | (uint)(rgba.x*255.0f);
}

float3 reflect(float3 I, float3 N) {
    return I - 2.0f * dot(N, I) * N;
}

float3 transformPosition(__constant float* transform, float3 pos) {
    float4 position = {pos.x, pos.y, pos.z, 1};
    float transformedPosition[4];

    // Multiply with transform
    // transform is column major
    for(int i = 0; i < 4; i++) {
        float sum = 0;
        sum += transform[0 + i*4]*position.x;
        sum += transform[1 + i*4]*position.y;
        sum += transform[2 + i*4]*position.z;
        sum += transform[3 + i*4]*position.w;
        transformedPosition[i] = sum;
    }

    float3 result = {transformedPosition[0], transformedPosition[1], transformedPosition[2]};
    return result;
}

float4 vload4_at_pos(int position, __constant float* data) {
    return (float4)(data[position], data[position+1], data[position+2], data[position+3]);
}

float4 getColorFromTransferFunction(float intensity, __constant float* transferFunction, int steps) {
    float4 first = vload4_at_pos(1, transferFunction);
    float firstIntensity = transferFunction[0];

    if(intensity <= firstIntensity)
        return first;

    for(int i = 1; i < steps; ++i) {
        float4 second = vload4_at_pos(i*5 + 1, transferFunction);
        float secondIntensity = transferFunction[i*5];
        if(intensity <= secondIntensity) {
            return mix(first, second, (intensity - firstIntensity)/(secondIntensity - firstIntensity));
        }
        first = second;
        firstIntensity = secondIntensity;
    }
    return first;
}

// Wang Hash based RNG, used to get rid of sampling artifact patterns
float ParallelRNG(unsigned int x) {
    unsigned int value = x;

    value = (value ^ 61) ^ (value>>16);
    value *= 9;
    value ^= value << 4;
    value *= 0x27d4eb2d;
    value ^= value >> 15;

    return (float)(value & 0x0ff) / 255.0f; // Convert to float 0-1
}


__kernel void volumeRender(
    __read_only image3d_t volume,
    __write_only image2d_t framebuffer,
    __constant float* invViewMatrix,
    __constant float* invViewMatrix2,
    __read_only image2d_t inputFramebuffer,
    __read_only image2d_t inputDepthFramebuffer,
    __private float zNear,
    __private float zFar,
    __constant float* transferFunction,
    __private int steps
    ) {

    const int width = get_image_width(framebuffer);
    const int height = get_image_height(framebuffer);

    // Grid position
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    // Normalized grid position
    const float u = ((x / (float) width)*2.0f-1.0f)*((float)width/height); // compensate for aspect ratio != 1
    const float v = ((y / (float) height)*2.0f-1.0f);

    const float4 inputColor = read_imagef(inputFramebuffer, volumeSampler, (int2)(x,y));

    // Bounding box of volume
    const float4 boxMin = (float4)(0.0f, 0.0f, 0.0f,1.0f);
    const float4 boxMax = (float4)(get_image_width(volume), get_image_height(volume), get_image_depth(volume), 1.0f);
    // Maximum depth to cast ray inside the volume

    // Calculate ray origin and direction
    float4 rayOrigin;
    float4 rayDirection;

    // Ray origin is at the camera center
    rayOrigin = (float4)(invViewMatrix[12], invViewMatrix[13], invViewMatrix[14], 1.0f);

    // Calculate ray direction which is the direction of the vector (u,v,-2) - (0,0,0)
    float4 temp = normalize(((float4)(u, v, -2.0f,0.0f)));
    // Apply camera rotation on the ray direction vector
    rayDirection.x = dot(temp, ((float4)(invViewMatrix[0],invViewMatrix[4],invViewMatrix[8], 0)));
    rayDirection.y = dot(temp, ((float4)(invViewMatrix[1],invViewMatrix[5],invViewMatrix[9], 0)));
    rayDirection.z = dot(temp, ((float4)(invViewMatrix[2],invViewMatrix[6],invViewMatrix[10], 0)));
    rayDirection.w = 1.0f;

    // Find the distance to where the ray hits the box
    float tnear, tfar;
    int hit = intersectBox(rayOrigin, rayDirection, boxMin, boxMax, &tnear, &tfar);
    if(!hit) {
        // Ray doesn't hit the box at all
        write_imagef(framebuffer, (int2)(x, y), inputColor);
        return;
    }

    if(tnear < 0.0f)
        tnear = 0.0f;     // clamp to near plane

    // Traverse along ray from back to front, while blending colors
    temp = inputColor;
    // Recover original depth from depth buffer: https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
    float depth = (read_imagef(inputDepthFramebuffer, volumeSampler, (int2)(x,y)).x*2.0f - 1.0f); // turn depth into normalized coordinate ([-1, 1]
    depth = 2.0f * zNear * zFar / (zFar + zNear - depth * (zFar - zNear));
    float distance = min(tfar, depth) + ParallelRNG(x + y*width);
    while(distance > tnear) { // back to front
        float4 pos = rayOrigin + rayDirection * distance;

        // read from 3D texture
        float sample = read_imagei(volume, volumeSampler, pos).x;

        // lookup intensity value in transfer function
        float4 color = getColorFromTransferFunction(sample, transferFunction, steps);

        // Shading: Calculate volume normal
        float3 normal;
        normal.x = 0.5f * (read_imagei(volume, volumeSampler, pos + (float4)(1, 0, 0, 0)).x -
                           read_imagei(volume, volumeSampler, pos - (float4)(1, 0, 0, 0)).x);
        normal.y = 0.5f * (read_imagei(volume, volumeSampler, pos + (float4)(0, 1, 0, 0)).x -
                           read_imagei(volume, volumeSampler, pos - (float4)(0, 1, 0, 0)).x);
        normal.z = 0.5f * (read_imagei(volume, volumeSampler, pos + (float4)(0, 0, 1, 0)).x -
                           read_imagei(volume, volumeSampler, pos - (float4)(0, 0, 1, 0)).x);
        normal = normalize(normal);
        normal = transformPosition(invViewMatrix2, normal);
        normal = normalize(normal);

        // Calculate color with light
        float3 objectColor = color.xyz;
        float3 lightColor = {0.7, 0.7, 0.7};
        float3 ambientColor = {0.2, 0.2, 0.2};
        float3 specularColor = {1, 1, 1};
        float3 LightPos = {0, 0, 0};
        float3 ViewPos = {0, 0, 0};
        float shininess = 16.0f;

        // TODO fix: Light direction is not entirely correct. Look at TriangleRenderer.vert
        LightPos = transformPosition(invViewMatrix2, LightPos);
        ViewPos = transformPosition(invViewMatrix2, ViewPos);

        float3 lightDir = normalize(LightPos - pos.xyz);
        float diff = max(dot(normal, lightDir), 0.0f);
        float3 diffuse = lightColor * (diff * objectColor);
        float3 viewDir = normalize(ViewPos - pos.xyz);
        float3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
        float3 specular = lightColor * (spec * specularColor);
        float3 ambient = lightColor * ambientColor;
        float3 result = ambient + diffuse + specular;
        color = (float4)(result.x, result.y, result.z, color.w);

        // accumulate result
        temp = mix(temp, color, color.w);

        distance -= 0.5f;
    }

    // write output color
    write_imagef(framebuffer, (int2)(x, y), temp);
}