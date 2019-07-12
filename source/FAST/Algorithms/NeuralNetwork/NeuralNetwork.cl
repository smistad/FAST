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
	__private int clipIntensity,
	__private int channelFirst
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
	const int width = get_global_size(0);
	const int height = get_global_size(1);
    if(channelFirst == 0) {
        int position = (pos.x + pos.y*width)*channels;
        output[position] = value.x;
        if(channels > 1)
            output[position+1] = value.y;
        if(channels > 2)
            output[position+2] = value.z;
        if(channels > 3)
            output[position+3] = value.w;
    } else {
        int position = pos.x + pos.y*width;
        output[position] = value.x;
        if(channels > 1)
            output[position + 1*width*height] = value.y;
        if(channels > 2)
            output[position + 2*width*height] = value.z;
        if(channels > 3)
            output[position + 3*width*height] = value.w;
    }
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
	__private int clipIntensity,
	__private int channelFirst
	) {

	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
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

    const int width = get_global_size(0);
	const int height = get_global_size(1);
	const int depth = get_global_size(2);
    if(channelFirst == 0) {
        int position = (pos.x + pos.y*width + pos.z*width*height)*channels;
        output[position] = value.x;
        if(channels > 1)
            output[position+1] = value.y;
        if(channels > 2)
            output[position+2] = value.z;
        if(channels > 3)
            output[position+3] = value.w;
    } else {
        int position = pos.x + pos.y*width + pos.z*width*height;
        output[position] = value.x;
        if(channels > 1)
            output[position + 1*width*height*depth] = value.y;
        if(channels > 2)
            output[position + 2*width*height*depth] = value.z;
        if(channels > 3)
            output[position + 3*width*height*depth] = value.w;
    }
}