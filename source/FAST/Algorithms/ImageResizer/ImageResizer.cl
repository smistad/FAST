__constant sampler_t samplerLinear = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
__constant sampler_t samplerNearest = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

void writeToImage(__write_only image2d_t input, int2 position, float4 value) {
    int dataType = get_image_channel_data_type(input);
	if(dataType == CLK_FLOAT) {
		write_imagef(input, position, value);
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
		write_imageui(input, position, convert_uint4(value));
	} else {
		write_imagei(input, position, convert_int4(value));
    }
}

float4 readFromImage(__read_only image2d_t input, sampler_t sampler, float2 position) {
    int dataType = get_image_channel_data_type(input);
	if(dataType == CLK_FLOAT) {
		return read_imagef(input, sampler, position);
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
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
	const float2 readPosition = {((float)position.x + 0.5f) / get_global_size(0), ((float)position.y + 0.5f) / (scale*get_image_height(input))};
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
	const float2 normalizedPosition = {(float)(position.x + 0.5f) / get_global_size(0), (float)(position.y + 0.5f) / get_global_size(1)};
	int dataType = get_image_channel_data_type(input);
	float4 value;
    if(useInterpolation == 1) {
        value = readFromImage(input, samplerLinear, normalizedPosition);
    } else {
        value = readFromImage(input, samplerNearest, normalizedPosition);
    }
    writeToImage(output, position, value);
}

// ================ 3D
#ifdef fast_3d_image_writes
void writeToImage3D(__write_only image3d_t input, int4 position, float4 value) {
    int dataType = get_image_channel_data_type(input);
	if(dataType == CLK_FLOAT) {
		write_imagef(input, position, value);
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
		write_imageui(input, position, convert_uint4(value));
	} else {
		write_imagei(input, position, convert_int4(value));
    }
}
#endif

float4 readFromImage3D(__read_only image3d_t input, sampler_t sampler, float4 position) {
    int dataType = get_image_channel_data_type(input);
	if(dataType == CLK_FLOAT) {
		return read_imagef(input, sampler, position);
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        return convert_float4(read_imageui(input, sampler, position));
	} else {
        return convert_float4(read_imagei(input, sampler, position));
    }
}

__kernel void resize3D(
		__read_only image3d_t input,
#ifdef fast_3d_image_writes
		__write_only image3d_t output,
#else
        __global void* output,
#endif
        __private char useInterpolation
    ) {
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	const float4 normalizedPosition = {
	        ((float)pos.x + 0.5f) / get_global_size(0),
            ((float)pos.y + 0.5f) / get_global_size(1),
            ((float)pos.z + 0.5f) / get_global_size(2),
            0
	};
	int dataType = get_image_channel_data_type(input);
	float4 value;
    if(useInterpolation == 1) {
        value = readFromImage3D(input, samplerLinear, normalizedPosition);
    } else {
        value = readFromImage3D(input, samplerNearest, normalizedPosition);
    }

#ifdef fast_3d_image_writes
    writeToImage3D(output, pos, value);
#else
    // TODO only one channel supported atm
    if(dataType == CLK_UNSIGNED_INT8) {
        ((__global uchar*)output)[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = value.x;
    } else if(dataType == CLK_SIGNED_INT8) {
        ((__global char*)output)[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = value.x;
    } else if(dataType == CLK_UNSIGNED_INT16) {
        ((__global ushort*)output)[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = value.x;
    } else if(dataType == CLK_SIGNED_INT16) {
        ((__global short*)output)[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = value.x;
    } else if(dataType == CLK_UNSIGNED_INT32) {
        ((__global uint*)output)[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = value.x;
    } else if(dataType == CLK_SIGNED_INT32) {
        ((__global int*)output)[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = value.x;
    } else if(dataType == CLK_FLOAT) {
        ((__global float*)output)[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = value.x;
    }
#endif
}