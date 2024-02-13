__const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;

__kernel void convert(
    __read_only image2d_t input,
    __write_only image2d_t output
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    int dataType = get_image_channel_data_type(input);
    if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        uint value = read_imageui(input, sampler, pos).x;
        write_imageui(output, pos, (uint4)(value, value, value, 255));
    } else if(dataType == CLK_FLOAT) {
        float value = read_imagef(input, sampler, pos).x;
        write_imagef(output, pos, (float4)(value, value, value, 1.0f));
    } else {
        int value = read_imagei(input, sampler, pos).x;
        write_imagei(output, pos, (int4)(value, value, value, 1));
    }
}