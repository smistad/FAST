__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void enhance(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __constant uchar* colormap
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    uchar3 color = vload3(read_imageui(input, sampler, pos).x, colormap);

    write_imageui(output, pos, (uint4)(color.x, color.y, color.z, 255));
}