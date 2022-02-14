__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

float calculateDiff(__read_only image2d_t imageInput, int2 pos1, int2 pos2, int filterSize, int iteration) {
    float res = 0.0f;
    for(int offset_x = -FILTER_SIZE; offset_x <= FILTER_SIZE; ++offset_x) {
        for(int offset_y = -FILTER_SIZE; offset_y <= FILTER_SIZE; ++offset_y) {
            int2 offset = {offset_x, offset_y};
            float diff = (float)read_imageui(imageInput, sampler, pos1 + offset).x/255.0f - (float)read_imageui(imageInput, sampler, pos2 + offset).x/255.0f;
            diff = diff*diff;
            res += diff;
        }
    }
    return res;
}

uchar findMedian(uchar array[], const int size) {
    for(uint i = 0; i < size; ++i) {
        for(uint j = 0; j < i; ++j) {
            if(array[i] > array[j]) {
                uchar tmp = array[i];
                array[i] = array[j];
                array[j] = tmp;
            }
        }
    }

    if(size % 2 == 0) {
        return (uchar)round((array[size / 2] + array[size / 2 + 1])*0.5f);
    } else {
        return array[size / 2];
    }
}

__kernel void preprocess(
        __read_only image2d_t input,
        __write_only image2d_t output
        ) { 
    const int2 pos = {get_global_id(0), get_global_id(1)};
    // Read neighborhood to thread
    uchar elements[25];
    int counter = 0;
    for(int a = -2; a <= 2; ++a) { 
        for(int b = -2; b <= 2; ++b) {
            elements[counter] = read_imageui(input, sampler, pos + (int2)(a,b)).x;
            ++counter;
        }
    }
    uchar median = findMedian(elements, 25);

    const float threshold = 150.0f; // TODO Set this threshold in a smarter way
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
    for(int searchOffsetX = -SEARCH_SIZE; searchOffsetX <= SEARCH_SIZE; ++searchOffsetX) {
        for(int searchOffsetY = -SEARCH_SIZE; searchOffsetY <= SEARCH_SIZE; ++searchOffsetY) {
            int2 searchOffsetAtScale = {searchOffsetX*(iteration+1), searchOffsetY*(iteration+1)};
            float diff = calculateDiff(imageInput, pos, pos + searchOffsetAtScale, filterSize, iteration);
            diff = native_exp(-diff/(2.0f*parameterH*parameterH));// / (2.0f*parameterH*parameterH);
            sumBottom += diff;
            sumTop += diff*read_imageui(imageInput, sampler, pos + searchOffsetAtScale).x/255.0f;
        }
    }

    write_imageui(imageOutput, pos, (uchar)round(clamp((sumTop/sumBottom)*255.0f, 0.0f, 255.0f)));
}

__kernel void multiplyWithInput(
            __read_only image2d_t inputImage,
            __read_only image2d_t NLMImage,
            __write_only image2d_t output,
            __private float multiplicationWeight
    ) {

    const int2 pos = {get_global_id(0), get_global_id(1)};
    float inputValue = read_imageui(inputImage, sampler, pos).x;
    float nlmValue = read_imageui(NLMImage, sampler, pos).x;

    write_imageui(output, pos, (uchar)round(sqrt((inputValue*multiplicationWeight + nlmValue*(1.0f-multiplicationWeight))*nlmValue)));
}
