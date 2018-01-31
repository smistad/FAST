__constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_NONE | CLK_FILTER_LINEAR;
__constant sampler_t sampler2 = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_LINEAR;

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

float4 readFromImage(__read_only image2d_t input, float2 position) {
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
    __private int newHeight
	) {
	const int2 position = {get_global_id(0), get_global_id(1)};
    const float scale = (float)newHeight / get_image_height(input);
	const float2 readPosition = {(float)position.x / get_global_size(0), (float)position.y / (scale*get_image_height(input))};
    if(position.y >= newHeight) {
        writeToImage(output, position, (float4)(0,0,0,0));
    } else {
        writeToImage(output, position, readFromImage(input, readPosition));
    }
}

__kernel void resize2D(
		__read_only image2d_t input,
		__write_only image2d_t output
    ) {
	const int2 position = {get_global_id(0), get_global_id(1)};
	const float2 normalizedPosition = {(float)position.x / get_global_size(0), (float)position.y / get_global_size(1)};
	int dataType = get_image_channel_data_type(input);
	if(dataType == CLK_FLOAT) {
		write_imagef(output, position, read_imagef(input, sampler, normalizedPosition));
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
		write_imageui(output, position, read_imageui(input, sampler, normalizedPosition));
	} else {
		write_imagei(output, position, read_imagei(input, sampler, normalizedPosition));
	}
}
