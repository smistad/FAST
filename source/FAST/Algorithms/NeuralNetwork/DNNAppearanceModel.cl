__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void imageNormalization(
	__read_only image2d_t input, // input has size target_width, input_height
	__write_only image2d_t output // output has size target_width, target_height
	) {
	
	const int2 pos = {get_global_id(0), get_global_id(1)};
	const int dataType = get_image_channel_data_type(input);
	const int target_height = get_image_height(output);

	if(pos.y < get_image_height(input)) {
		// If current y pos is small than input height, use input pixel, because we are inside image
		float value;
		if(dataType == CLK_FLOAT) {
			value = read_imagef(input, sampler, pos).x;
		} else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16) {
			value = read_imagei(input, sampler, pos).x;
		} else {
			value = read_imageui(input, sampler, pos).x;
		}	
		
		write_imagef(output, pos, value / 255);
	} else { 
		// If current y pos is larger than input height, set 0, because we are outside image
		// This means that target_height > input_height
		write_imagef(output, pos, 0);
	}	

}
	