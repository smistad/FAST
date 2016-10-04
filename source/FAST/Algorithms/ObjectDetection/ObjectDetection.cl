__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void imageNormalization(
	__read_only image2d_t input, // input has size 256, 256
	__write_only image2d_t outputLeft, // output has size target_width, 256
	__write_only image2d_t outputRight // output has size target_width, 256
	) {
	
	const int2 pos = {get_global_id(0), get_global_id(1)};
	const int dataType = get_image_channel_data_type(input);
	const int target_height = get_image_height(outputLeft);

	if(pos.y < get_image_height(input)) {
		// If current y pos is small than input height, use input pixel, because we are inside image
		float valueLeft;
		float valueRight;
		if(dataType == CLK_FLOAT) {
			valueLeft = read_imagef(input, sampler, pos).x;
			valueRight = read_imagef(input, sampler, pos + (int2)(256-192, 0)).x;
		} else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16) {
			valueLeft = read_imagei(input, sampler, pos).x;
			valueRight = read_imagei(input, sampler, pos + (int2)(256-192, 0)).x;
		} else {
			valueLeft = read_imageui(input, sampler, pos).x;
			valueRight = read_imageui(input, sampler, pos + (int2)(256-192, 0)).x;
		}
		
		write_imagef(outputLeft, pos, valueLeft / 255);
		write_imagef(outputRight, pos, valueRight / 255);
	} else {
		// If current y pos is larger than input height, set 0, because we are outside image
		// This means that target_height > input_height
		write_imagef(outputLeft, pos, 0);
		write_imagef(outputRight, pos, 0);
	}

}
	