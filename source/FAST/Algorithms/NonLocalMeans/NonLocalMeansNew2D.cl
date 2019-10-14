__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

float calculateDiff(__read_only image2d_t imageInput, int2 pos1, int2 pos2) {
    float res = 0.0f;
    for(int offset_x = -1; offset_x <= 1; ++offset_x) {
        for(int offset_y = -1; offset_y <= 1; ++offset_y) {
            int2 offset = {offset_x, offset_y};
            float diff = (float)read_imageui(imageInput, sampler, pos1 + offset).x/255.0f - (float)read_imageui(imageInput, sampler, pos2 + offset).x/255.0f;
            //printf("das: %f %d %d %d\n", diff, offset.x, offset.y, read_imageui(imageInput, sampler, pos2 + offset).x);
            diff = diff*diff;
            res += diff;
        }
    }
    return res;
}

__kernel void preprocess(
        __read_only image2d_t input,
        __write_only image2d_t output
        ) { 
    const int2 pos = {get_global_id(0), get_global_id(1)};
    // Find median element
    uchar elements[25];
    int counter = 0;
    for(int a = -2; a <= 2; ++a) { 
        for(int b = -2; b <= 2; ++b) {
            elements[counter] = read_imageui(input, sampler, pos + (int2)(a,b)).x;
            ++counter;
        }
    }
    uchar median;
    float bestSum = FLT_MAX;
    for(int i = 0; i < 25; ++i) {
        float sum = 0.0f;
        for(int j = 0; j < 25; ++j) {
            sum += (elements[i]-elements[j])*(elements[i]-elements[j]);
        }
        if(sum < bestSum) {
            bestSum = sum;
            median = elements[i];
        }
    }

    const float threshold = 150.0f;
    const float current = read_imageui(input, sampler, pos).x;
    uchar newPixel = current - max((float)median - current - threshold, 0.0f);
    write_imageui(output, pos, newPixel);
}

__kernel void nonLocalMeansFilter(
        __read_only image2d_t imageInput,
        __write_only image2d_t imageOutput,
        __private int searchSize,
        __private int filterSize,
        __private float parameterH,
        __private int iteration
        ) { 
    const int2 pos = {get_global_id(0), get_global_id(1)};
    float sumBottom = 0.0f;
    float sumTop = 0.0f;

    // Loop over search region
    for(int searchOffsetX = -searchSize; searchOffsetX <= searchSize; ++searchOffsetX) {
        for(int searchOffsetY = -searchSize; searchOffsetY <= searchSize; ++searchOffsetY) {
            int2 searchOffsetAtScale = {searchOffsetX*(iteration+1), searchOffsetY*(iteration+1)};

            float diff = calculateDiff(imageInput, pos, pos + searchOffsetAtScale);
            //printf("diff: %f %d %d\n", diff, searchOffsetAtScale.x, searchOffsetAtScale.y);
            diff = exp(-diff/(2.0f*parameterH*parameterH));// / (2.0f*parameterH*parameterH);
            sumBottom += diff;
            sumTop += diff*read_imageui(imageInput, sampler, pos + searchOffsetAtScale).x/255.0f;
        }
    }

    write_imageui(imageOutput, pos, (uchar)clamp((sumTop/sumBottom)*255.0f, 0.0f, 255.0f ));
}
