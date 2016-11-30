__kernel void fillImage2D(
		__write_only image2d_t image,
		__private float value
		) {
	const int2 pos = {get_global_id(0), get_global_id(1)};
	int dataType = get_image_channel_data_type(image);
	if(dataType == CLK_FLOAT) {
		write_imagef(image, pos, (float4)(value, value, value, value));
	} else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16) {
		write_imagei(image, pos, (int4)(value, value, value, value));
	} else {
		write_imageui(image, pos, (uint4)(value, value, value, value));
	}
}

/*
__kernel void fillImage3D(
		__write_only image3d_t image,
		float value) {
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	int dataType = get_image_channel_data_type(image);
	if(dataType == CLK_FLOAT) {
		write_imagef(image, pos, (float4)(value, value, value, value));
	} else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16) {
		write_imagei(image, pos, (int4)(value, value, value, value));
	} else {
		write_imageui(image, pos, (uint4)(value, value, value, value));
	}
}
*/