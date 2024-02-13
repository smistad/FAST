__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

float4 readImageAsFloat2D(__read_only image2d_t image, sampler_t sampler, int2 position) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        return read_imagef(image, sampler, position);
    } else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT32) {
        return convert_float4(read_imagei(image, sampler, position));
    } else {
        return convert_float4(read_imageui(image, sampler, position));
    }
}

void writeImageAsFloat2D(__write_only image2d_t image, int2 position, float4 value) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        write_imagef(image, position, value);
    } else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT32) {
        write_imagei(image, position, convert_int4(round(value)));
    } else {
        write_imageui(image, position, convert_uint4(round(value)));
    }
}

__kernel void cast2D(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float scaleFactor
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    const int dataType = get_image_channel_data_type(output);

    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        writeImageAsFloat2D(output, pos, readImageAsFloat2D(input, sampler, pos)*scaleFactor);
    } else {
        writeImageAsFloat2D(output, pos, round(readImageAsFloat2D(input, sampler, pos)*scaleFactor));
    }
}
