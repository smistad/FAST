
__kernel void drawFilledCircles(
    __global float2* centroids,
    __private int size,
    __write_only image2d_t output,
    __global float2* radii,
    __private char singleRadius,
    __private float value,
    __private float3 color
    ) {
    const int index = get_global_id(0);
    float2 centroid = centroids[index];
    float2 radius;
    if(singleRadius == 1) {
        radius = radii[0];
    } else {
        radius = radii[index];
    }

    const int dataType = get_image_channel_data_type(output);

    float4 write = {value, value, value, value};
    if(value == 0.0f) {
        // Use color instead
        write.xyz = color;
        if(dataType != CLK_FLOAT) {
            write = round(write*255);
        }
    }

    for(int a = -ceil(radius.x); a <= ceil(radius.x); ++a) {
        for(int b = -ceil(radius.y); b <= ceil(radius.y); ++b) {
            float2 current = round(centroid + (float2)(a, b));
            float2 n = current - centroid;
            float inside = (n.x*n.x)/(radius.x*radius.x) + (n.y*n.y)/(radius.y*radius.y);
            if(inside <= 1.0f) { // Inside ellipse
                int2 pos = convert_int2(current);
                if(pos.x >= 0 && pos.y >= 0 && pos.x < get_image_width(output) && pos.y < get_image_height(output)) {
                    if(dataType == CLK_FLOAT) {
                        write_imagef(output, pos, write);
                    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
                        write_imageui(output, pos, convert_uint4(write));
                    } else {
                        write_imagei(output, pos, convert_int4(write));
                    }
                }
            }
        }
    }
}

__kernel void drawCircles(
    __global float2* centroids,
    __private int size,
    __write_only image2d_t output,
    __global float2* radii,
    __private char singleRadius,
    __private float value,
    __private float3 color
    ) {
    const int index = get_global_id(0);
    float2 centroid = centroids[index];
    float2 radius;
    if(singleRadius == 1) {
        radius = radii[0];
    } else {
        radius = radii[index];
    }
    const int dataType = get_image_channel_data_type(output);

    float4 write = {value, value, value, value};
    if(value == 0.0f) {
        // Use color instead
        write.xyz = color;
        if(dataType != CLK_FLOAT) {
            write = round(write*255);
        }
    }

    // Calculate circumference
    const int N = ceil(2*M_PI_F*max(radius.x, radius.y)); // No closed formula for ellipse circumference
    for(int i = 0; i < N; ++i) {
        float angle = i*2.0f*M_PI_F/N;
        int x = centroid.x + round(cos(angle)*radius.x);
        int y = centroid.y + round(sin(angle)*radius.y);
        printf("%d %d\n", x, y);

        if(dataType == CLK_FLOAT) {
            write_imagef(output, (int2)(x, y), write);
        } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
            write_imageui(output, (int2)(x, y), convert_uint4(write));
        } else {
            write_imagei(output, (int2)(x, y), convert_int4(write));
        }
    }

}