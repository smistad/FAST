__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void invert2D(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float min,
        __private float max
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    int dataType = get_image_channel_data_type(input);

    float4 value;
    if(dataType == CLK_FLOAT) {
        value = read_imagef(input, sampler, pos);
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        value = convert_float4(read_imageui(input, sampler, pos));
    } else {
        value = convert_float4(read_imagei(input, sampler, pos));
    }
    value = (max - min) - value;
    if(dataType == CLK_FLOAT) {
        write_imagef(output, pos, value);
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        write_imageui(output, pos, convert_uint4(value));
    } else {
        write_imagei(output, pos, convert_int4(value));
    }
}
