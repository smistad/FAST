#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

#ifdef cl_khr_3d_image_writes
__kernel void gaussianSmoothing(
        __read_only image3d_t input,
        __constant float * mask,
        __write_only image3d_t output,
        __private unsigned char maskSize
        ) {

    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const unsigned char halfSize = (maskSize-1)/2;

    float sum = 0.0f;
    for(int x = -halfSize; x <= halfSize; x++) {
    for(int y = -halfSize; y <= halfSize; y++) {
    for(int z = -halfSize; z <= halfSize; z++) {
        const int4 offset = {x,y,z,0};
        const uint maskOffset = x+halfSize+(y+halfSize)*maskSize+(z+halfSize)*maskSize*maskSize;
#ifdef TYPE_FLOAT
        sum += mask[maskOffset]*read_imagef(input, sampler, pos+offset).x;
#elif TYPE_UINT
        sum += mask[maskOffset]*read_imageui(input, sampler, pos+offset).x;
#else
        sum += mask[maskOffset]*read_imagei(input, sampler, pos+offset).x;
#endif
    }}}

#ifdef TYPE_FLOAT
    write_imagef(output, pos, sum);
#elif TYPE_UINT
    write_imageui(output, pos, round(sum));
#else
    write_imagei(output, pos, round(sum));
#endif
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
    for(int x = -halfSize; x <= halfSize; x++) {
    for(int y = -halfSize; y <= halfSize; y++) {
    for(int z = -halfSize; z <= halfSize; z++) {
        const int4 offset = {x,y,z,0};
        const uint maskOffset = x+halfSize+(y+halfSize)*maskSize+(z+halfSize)*maskSize*maskSize;
#ifdef TYPE_FLOAT
        sum += mask[maskOffset]*read_imagef(input, sampler, pos+offset).x;
#elif TYPE_UINT
        sum += mask[maskOffset]*read_imageui(input, sampler, pos+offset).x;
#else
        sum += mask[maskOffset]*read_imagei(input, sampler, pos+offset).x;
#endif
    }}}

    output[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = sum;
}
#endif
