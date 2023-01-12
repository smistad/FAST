__const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;

__kernel void writeSegmentation(
        __write_only image2d_t current,
        __read_only image2d_t new,
        int label
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    if(read_imageui(new, sampler, pos).x > 0) {
        write_imageui(current, pos, label);
    }
}