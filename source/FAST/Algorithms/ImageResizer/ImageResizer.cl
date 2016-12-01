__constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_NONE | CLK_FILTER_LINEAR;

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

