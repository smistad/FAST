__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void normalize2DInput(
	__read_only image2d_t input,
	__global float* output,
	__private float scaleFactor,
	__private float mean,
	__private float std,
	__private int signedInputNormalization,
	__private int horizontalFlip,
	__private int channels,
	__private float minIntensity,
	__private float maxIntensity,
	__private int clipIntensity
	) {
	
	const int2 pos = {get_global_id(0), get_global_id(1)};
	const int dataType = get_image_channel_data_type(input);
	float4 value;
	if(dataType == CLK_FLOAT) {
		value = read_imagef(input, sampler, pos);
	} else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16) {
		value = convert_float4(read_imagei(input, sampler, pos));
	} else {
		value = convert_float4(read_imageui(input, sampler, pos));
	}

	if(clipIntensity)
	    value = clamp(value, minIntensity, maxIntensity);
	value = (value - mean)/std;
    value = value*scaleFactor;
    if(signedInputNormalization) {
        value = value*2 - 1;
	}

    int x;
	if(horizontalFlip == 1) {
		x = (get_global_size(0) - pos.x - 1);
	} else {
		x = pos.x;
	}
    int position = (pos.x + pos.y*get_global_size(0))*channels;
    output[position] = value.x;
    if(channels > 1)
        output[position+1] = value.y;
    if(channels > 2)
        output[position+2] = value.z;
    if(channels > 3)
        output[position+3] = value.w;
}

__kernel void normalize3DInput(
	__read_only image3d_t input,
	__global float* output,
	__private float scaleFactor,
    __private float mean,
	__private float std,
	__private int signedInputNormalization,
	__private int horizontalFlip,
	__private int channels,
	__private float minIntensity,
	__private float maxIntensity,
	__private int clipIntensity
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

	if(clipIntensity)
	    value = clamp(value, minIntensity, maxIntensity);
	value = (value - mean)/std;
    value = value*scaleFactor;
    if(signedInputNormalization) {
        value = value*2 - 1;
	}

    int position = pos.x + pos.y*get_global_size(0) + pos.z*get_global_size(0)*get_global_size(1);
    output[position] = value;
}