__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void gaussianSmoothing(
        __read_only image2d_t input,
        __constant float * mask,
        __write_only image2d_t output,
        __private unsigned char maskSize
        ) {

    const int2 pos = {get_global_id(0), get_global_id(1)};
    const unsigned char halfSize = (maskSize-1)/2;

    float sum = 0.0f;
    int dataType = get_image_channel_data_type(input);
    for(int x = -halfSize; x <= halfSize; x++) {
    for(int y = -halfSize; y <= halfSize; y++) {
        const int2 offset = {x,y};
        if(dataType == CLK_FLOAT) {
            sum += mask[x+halfSize+(y+halfSize)*maskSize]*read_imagef(input, sampler, pos+offset).x;
        } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            sum += mask[x+halfSize+(y+halfSize)*maskSize]*read_imageui(input, sampler, pos+offset).x;
        } else {
            sum += mask[x+halfSize+(y+halfSize)*maskSize]*read_imagei(input, sampler, pos+offset).x;
        }
    }}

    int outputDataType = get_image_channel_data_type(output);
    if(outputDataType == CLK_FLOAT) {
        write_imagef(output, pos, sum);
    } else if(outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(output, pos, round(sum));
    } else {
        write_imagei(output, pos, round(sum));
    }
}
