__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)

__kernel void convertToHU(
        __read_only image3d_t input,
#ifdef fast_3d_image_writes
        __write_only image3d_t output
#else
        __global short* output
#endif
) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    int value = read_imageui(input, sampler, pos).x;
    value -= 1024;
#ifdef fast_3d_image_writes
    write_imagei(output, pos, value);
#else
    output[LPOS(pos)] = value;
#endif
}
