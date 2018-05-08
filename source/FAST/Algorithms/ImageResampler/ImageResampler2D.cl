__constant sampler_t samplerLinear = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
__constant sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

float readImageAsFloat2D(__read_only image2d_t image, sampler_t sampler, float2 position) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        return read_imagef(image, sampler, position).x;
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        return (float)read_imagei(image, sampler, position).x;
    } else {
        return (float)read_imageui(image, sampler, position).x;
    }
}

__kernel void resample2D(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float scaleX,
        __private float scaleY,
        __private uchar useInterpolation
) {
    const int2 outputPosition = {get_global_id(0), get_global_id(1)};
    const int2 size = {get_global_size(0), get_global_size(1)};
    float2 inputPosition = {outputPosition.x*(1.0f/scaleX), outputPosition.y*(1.0f/scaleY)};

    int dataType = get_image_channel_data_type(output);
    float value;
    if(useInterpolation == 1) {
        value = readImageAsFloat2D(input, samplerLinear, inputPosition);
    } else {
        value = readImageAsFloat2D(input, samplerNearest, inputPosition);
    }
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        write_imagef(output, outputPosition, value);
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        write_imagei(output, outputPosition, round(value));
    } else {
        write_imageui(output, outputPosition, round(value));
    }
}
