const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;

__kernel void flip2D(
    __read_only image2d_t input,
    __write_only image2d_t output,
    __private char flipHorizontal,
    __private char flipVertical
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    int2 targetPos = pos;
    if(flipHorizontal == 1) 
        targetPos.x = get_global_size(0) - pos.x - 1;
    if(flipVertical == 1) 
        targetPos.y = get_global_size(1) - pos.y - 1;
    int dataType = get_image_channel_data_type(input);
    if(dataType == CLK_FLOAT) {
        write_imagef(output, targetPos, read_imagef(input, sampler, pos));
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        write_imageui(output, targetPos, read_imageui(input, sampler, pos));
    } else {
        write_imagei(output, targetPos, read_imagei(input, sampler, pos));
    }
}
