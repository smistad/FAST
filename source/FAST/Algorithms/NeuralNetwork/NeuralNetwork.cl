__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void subtractMeanImage(
	__read_only image2d_t input,
	__read_only image2d_t meanImage,
	__write_only image2d_t output
	) {
	
	const int2 pos = {get_global_id(0), get_global_id(1)};
	const int dataType = get_image_channel_data_type(input);
	float value;
	if(dataType == CLK_FLOAT) {
		value = read_imagef(input, sampler, pos).x;
	} else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16) {
		value = read_imagei(input, sampler, pos).x;
	} else {
		value = read_imageui(input, sampler, pos).x;
	}
	value -= read_imagef(meanImage, sampler, pos).x;
	
	write_imagef(output, pos, value);
}
	