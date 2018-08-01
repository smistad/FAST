__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void normalize2DInput(
	__read_only image2d_t input,
	__global float* output,
	__private float scaleFactor,
	__private int signedInputNormalization,
	__private int horizontalFlip
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

    value = value*scaleFactor;
    if(signedInputNormalization) {
        value = value*2 - 1;
	}

	if(horizontalFlip == 1) {
        output[(get_global_size(0) - pos.x - 1) + pos.y*get_global_size(0)] = value;
    } else {
        output[pos.x + pos.y*get_global_size(0)] = value;
    }
}

__kernel void normalize3DInput(
	__read_only image3d_t input,
	__global float* output,
	__private float scaleFactor,
	__private int signedInputNormalization
	) {

	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	const int dataType = get_image_channel_data_type(input);
	float value;
	if(dataType == CLK_FLOAT) {
		value = read_imagef(input, sampler, pos).x;
	} else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16) {
		value = read_imagei(input, sampler, pos).x;
	} else {
		value = read_imageui(input, sampler, pos).x;
	}

    value = value*scaleFactor;
    if(signedInputNormalization) {
        value = value*2 - 1;
	}

    output[pos.x + pos.y*get_global_size(0) + pos.z*get_global_size(0)*get_global_size(1)] = value;
}