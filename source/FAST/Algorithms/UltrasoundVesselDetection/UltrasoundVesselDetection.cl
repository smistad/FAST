__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
__constant sampler_t samplerInterpolation = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

// This is missing on some AMD platforms for some reason
#define M_PI 3.14159265358979323846

bool outOfBounds(int2 pos, float radius, uint width, uint height) {
    return pos.x-radius < 0 || pos.y-radius < 0 ||
            pos.x+radius >= width || pos.y+radius >= height;
}

__kernel void vesselDetection(
        __read_only image2d_t image,
        __read_only image2d_t gradients,
        __write_only image2d_t result,
        float spacing
        ) {
    
    const float radiusMinInMM = 3.5;
    const float radiusMaxInMM = 6;
    const float scaleStep = 0.1;
    const uint samples = 32;
    const float radiusMinInPixels = radiusMinInMM*(1.0f/spacing);
    const float radiusMaxInPixels = radiusMaxInMM*(1.0f/spacing);
    const float radiusStepInPixels = 2;

    const int2 pos = {get_global_id(0)*4, get_global_id(1)*4};
    
    // Should be 10 pixel  max within image
    if(outOfBounds(pos, 10, get_image_width(image), get_image_height(image))) {
        write_imagef(result, pos, (float4)(0,0,0,0));
        return;
    }
    
    // For each radius
    float bestFitness = -100;
    float bestRadius = 0;
    float bestScale = 0;
    for(float scale = 1.0; scale >= 0.5; scale -= scaleStep) {
        float sumLumenIntensity = 0;
        uchar radiusCounter = 0;
        for(float radius = 2; radius <= radiusMaxInPixels; radius += radiusStepInPixels) {
        //for(float radius = radiusMinInPixels; radius <= radiusMaxInPixels; radius += radiusStepInPixels) {
            float fitness = 0;
            float intensity = 0;
            // For each point on circle
            for(uint i = 0; i < samples; i++) {
                const float alpha = (float)(2.0f*M_PI*i)/samples;
                float2 direction = {cos(alpha), sin(alpha)*scale};
                const float2 samplePos = direction*radius + convert_float2(pos);
                float2 gradient = read_imagef(gradients, samplerInterpolation, samplePos).xy;
                //gradient = normalize(gradient); // only direction matter
                // Calculate normal
                float2 normal = {
                        scale*radius*cos(alpha),//(samplePos.x-pos.x)*scale,
                        radius*sin(alpha)//(samplePos.y-pos.y)*(1.0f/scale)
                };
                normal = normalize(normal);
                fitness += dot(gradient, normal);
                intensity += read_imageui(image, samplerInterpolation, samplePos).x;
            }
            float averageLumenIntensity = sumLumenIntensity / (radiusCounter*samples);
            fitness = fitness / samples;
            if(fitness > bestFitness && radius > radiusMinInPixels) {
                // Calculate border intensity
                float averageBorderIntensity = 0.0f;
                for(uint i = 0; i < samples; i++) {
                    const float alpha = (float)(2.0f*M_PI*i)/samples;
                    float2 direction = {cos(alpha), sin(alpha)*scale};
                    const float2 samplePos = direction*(radius+radiusStepInPixels) + convert_float2(pos);
                    averageBorderIntensity += read_imageui(image, samplerInterpolation, samplePos).x;
                }
                averageBorderIntensity /= samples;
                if((averageBorderIntensity-averageLumenIntensity)/averageBorderIntensity > 0.5) { // the percentage of the intensity difference
                    bestFitness = fitness;
                    bestRadius = radius;
                    bestScale = scale;
                }
            }
            radiusCounter++;
            sumLumenIntensity += intensity; 
        } // end for each radius

    } // end for each scale
    
    // Store best fit
    write_imagef(result, pos, (float4)(bestFitness, bestRadius, bestScale, pos.x+pos.y*get_image_width(image)));
}

__kernel void createSegmentation(
        __write_only image2d_t output,
        __private float posX,
        __private float posY,
        __private float majorRadius,
        __private float minorRadius
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    
    float a = majorRadius;
    float b = minorRadius;
    float x = pos.x - posX;
    float y = pos.y - posY;
    
    float test = pow(x / a, 2.0f) + pow(y / b, 2.0f);
    
    if(test < 1) {
        write_imageui(output, pos, 3);
    }
    
}   
