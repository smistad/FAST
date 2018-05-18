__constant sampler_t samplerLinear = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_NONE | CLK_FILTER_LINEAR;
__constant sampler_t samplerNearest = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

void writeToImage(__write_only image2d_t input, int2 position, float4 value) {
    int dataType = get_image_channel_data_type(input);
	if(dataType == CLK_FLOAT) {
		write_imagef(input, position, value);
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
		write_imageui(input, position, convert_uint4(value));
	} else {
		write_imagei(input, position, convert_int4(value));
    }
}

float4 readFromImage(__read_only image2d_t input, sampler_t sampler, float2 position) {
    int dataType = get_image_channel_data_type(input);
	if(dataType == CLK_FLOAT) {
		return read_imagef(input, sampler, position);
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        return convert_float4(read_imageui(input, sampler, position));
	} else {
        return convert_float4(read_imagei(input, sampler, position));
    }
}

__kernel void resize2DpreserveAspect(
	__read_only image2d_t input,
	__write_only image2d_t output,
    __private int newHeight,
    __private char useInterpolation
	) {
	const int2 position = {get_global_id(0), get_global_id(1)};
    const float scale = (float)newHeight / get_image_height(input);
	const float2 readPosition = {(float)position.x / get_global_size(0), (float)position.y / (scale*get_image_height(input))};
	float4 value;
    if(useInterpolation == 1) {
        value = readFromImage(input, samplerLinear, readPosition);
    } else {
        value = readFromImage(input, samplerNearest, readPosition);
    }
    if(position.y >= newHeight) {
        writeToImage(output, position, (float4)(0,0,0,0));
    } else {
        writeToImage(output, position, value);
    }
}

__kernel void resize2D(
		__read_only image2d_t input,
		__write_only image2d_t output,
        __private char useInterpolation
    ) {
	const int2 position = {get_global_id(0), get_global_id(1)};
	const float2 normalizedPosition = {(float)position.x / get_global_size(0), (float)position.y / get_global_size(1)};
	int dataType = get_image_channel_data_type(input);
	float4 value;
    if(useInterpolation == 1) {
        value = readFromImage(input, samplerLinear, normalizedPosition);
    } else {
        value = readFromImage(input, samplerNearest, normalizedPosition);
    }
    writeToImage(output, position, value);
}
