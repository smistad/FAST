__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

float4 readImageAsFloat2D(__read_only image2d_t image, sampler_t sampler, int2 position) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        return read_imagef(image, sampler, position);
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        return convert_float4(read_imagei(image, sampler, position));
    } else {
        return convert_float4(read_imageui(image, sampler, position));
    }
}


void writeImageAsFloat2D(__write_only image2d_t image, int2 position, float4 value) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        write_imagef(image, position, value);
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        write_imagei(image, position, convert_int4(round(value)));
    } else {
        write_imageui(image, position, convert_uint4(round(value)));
    }
}

float4 readImageAsFloat3D(__read_only image3d_t image, sampler_t sampler, int4 position) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        return read_imagef(image, sampler, position);
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        return convert_float4(read_imagei(image, sampler, position));
    } else {
        return convert_float4(read_imageui(image, sampler, position));
    }
}

__kernel void multiply3D(
        __read_only image3d_t input1,
        __read_only image3d_t input2,
        __global DATA_TYPE* output,
        __private uint outputChannels
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    float4 value1 = readImageAsFloat3D(input1, sampler, pos);
    float4 value2 = readImageAsFloat3D(input2, sampler, pos);
    float4 value = value1*value2;

    output[(pos.x + pos.y*get_image_width(input1) + pos.z*get_image_width(input1)*get_image_height(input1))*outputChannels] = value.x;
    if(outputChannels > 1)
        output[(pos.x + pos.y*get_image_width(input1) + pos.z*get_image_width(input1)*get_image_height(input1))*outputChannels + 1] = value.y;
    if(outputChannels > 2)
        output[(pos.x + pos.y*get_image_width(input1) + pos.z*get_image_width(input1)*get_image_height(input1))*outputChannels + 2] = value.z;
    if(outputChannels > 3)
        output[(pos.x + pos.y*get_image_width(input1) + pos.z*get_image_width(input1)*get_image_height(input1))*outputChannels + 3] = value.w;
}

__kernel void multiply2D(
        __read_only image2d_t input1,
        __read_only image2d_t input2,
        __write_only image2d_t output,
        __private uint input1Channels,
        __private uint input2Channels,
        __private uint outputChannels
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float4 value1 = readImageAsFloat2D(input1, sampler, pos);
    float4 value2 = readImageAsFloat2D(input2, sampler, pos);
    float4 value;
    if(input1Channels == input2Channels) {
        value = value1*value2;
    } else if(input1Channels == 1) {
        value = value1.x*value2;
    } else if(input2Channels == 1) {
        value = value1*value2.x;
    }
    writeImageAsFloat2D(output, pos, value);
}
