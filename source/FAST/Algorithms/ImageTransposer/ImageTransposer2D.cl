const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;

__kernel void transpose2D(
    __read_only image2d_t input,
    __write_only image2d_t output
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    int dataType = get_image_channel_data_type(input);
    if(dataType == CLK_FLOAT) {
        write_imagef(output, pos.yx, read_imagef(input, sampler, pos));
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        write_imageui(output, pos.yx, read_imageui(input, sampler, pos));
    } else {
        write_imagei(output, pos.yx, read_imagei(input, sampler, pos));
    }
}