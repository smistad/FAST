__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

#ifdef fast_3d_image_writes
__kernel void gaussianSmoothing(
        __read_only image3d_t input,
        __constant float * mask,
        __write_only image3d_t output,
        __private unsigned char maskSize,
        __private unsigned char direction
        ) {

    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const unsigned char halfSize = (maskSize-1)/2;

    float sum = 0.0f;
    int dataType = get_image_channel_data_type(input);
    for(int i = -halfSize; i <= halfSize; ++i) {
        int4 offset = {0,0,0,0};
        if(direction == 0) {
            offset.x = i;
        } else if(direction == 1) {
            offset.y = i;
        } else {
            offset.z = i;
        }
        const uchar maskOffset = halfSize + i;
        if(dataType == CLK_FLOAT) {
            sum += mask[maskOffset]*read_imagef(input, sampler, pos+offset).x;
        } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            sum += mask[maskOffset]*read_imageui(input, sampler, pos+offset).x;
        } else {
            sum += mask[maskOffset]*read_imagei(input, sampler, pos+offset).x;
        }
    }

    int outputDataType = get_image_channel_data_type(output);
    if(outputDataType == CLK_FLOAT) {
        write_imagef(output, pos, sum);
    } else if(outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(output, pos, round(sum));
    } else {
        write_imagei(output, pos, round(sum));
    }
}

#else
__kernel void gaussianSmoothing(
        __read_only image3d_t input,
        __constant float * mask,
        __global TYPE* output,
        __private unsigned char maskSize
        ) {

    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const unsigned char halfSize = (maskSize-1)/2;

    float sum = 0.0f;
    int dataType = get_image_channel_data_type(input);
    for(int x = -halfSize; x <= halfSize; x++) {
    for(int y = -halfSize; y <= halfSize; y++) {
    for(int z = -halfSize; z <= halfSize; z++) {
        const int4 offset = {x,y,z,0};
        const uint maskOffset = x+halfSize+(y+halfSize)*maskSize+(z+halfSize)*maskSize*maskSize;
        if(dataType == CLK_FLOAT) {
            sum += mask[maskOffset]*read_imagef(input, sampler, pos+offset).x;
        } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            sum += mask[maskOffset]*read_imageui(input, sampler, pos+offset).x;
        } else {
            sum += mask[maskOffset]*read_imagei(input, sampler, pos+offset).x;
        }
    }}}

    output[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = sum;
}
#endif
