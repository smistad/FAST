__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void clip2D(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float min,
        __private float max
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    const int dataType = get_image_channel_data_type(input);

    if(dataType == CLK_FLOAT) {
        write_imagef(output, pos, clamp(read_imagef(input, sampler, pos), min, max));
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        write_imageui(output, pos, clamp(read_imageui(input, sampler, pos), (uint)min, (uint)max));
    } else {
        write_imagei(output, pos, clamp(read_imagei(input, sampler, pos), (int)min, (int)max));
    }
}
