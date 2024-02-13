__const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;

__kernel void convert(
    __read_only image2d_t input,
    __write_only image2d_t output
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    int dataType = get_image_channel_data_type(input);
    if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        uint4 value = read_imageui(input, sampler, pos);
        uint average = round(((float)(value.x + value.y + value.z))/3.0f);
        write_imageui(output, pos, average);
    } else if(dataType == CLK_FLOAT) {
        float4 value = read_imagef(input, sampler, pos);
        float average = value.x + value.y + value.z;
        write_imagef(output, pos, average);
    } else {
        int4 value = read_imagei(input, sampler, pos);
        int average = round(((float)(value.x + value.y + value.z))/3.0f);
        write_imagei(output, pos, average);
    }
}
