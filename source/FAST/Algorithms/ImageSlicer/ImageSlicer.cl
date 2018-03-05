__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void orthogonalSlicing(
        __read_only image3d_t input,
        __write_only image2d_t output,
        __private int slice,
        __private int slicePlane
        ) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    int4 pos;
    if(slicePlane == 0) {
        pos = (int4)(slice,x,y,0);
    } else if(slicePlane == 1) {
        pos = (int4)(x,slice,y,0);
    } else {
        pos = (int4)(x,y,slice,0);
    }

    int dataType = get_image_channel_data_type(input);
    if(dataType == CLK_FLOAT) {
		float4 value = read_imagef(input, sampler, pos);
		write_imagef(output, (int2)(x,y), value);
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
		uint4 value = read_imageui(input, sampler, pos);
		write_imageui(output, (int2)(x,y), value);
	} else {
		int4 value = read_imagei(input, sampler, pos);
		write_imagei(output, (int2)(x,y), value);
    }
}

float4 transformPosition(__constant float* transform, int2 PBOposition) {
    float4 position = {PBOposition.x, PBOposition.y, 0, 1};
    float transformedPosition[4];
    //printf("PBO pos: %d %d\n", PBOposition.x, PBOposition.y);

    // Multiply with transform
    // transform is column major
    for(int i = 0; i < 4; i++) {
        float sum = 0;
        sum += transform[i + 0*4]*position.x;
        sum += transform[i + 1*4]*position.y;
        sum += transform[i + 2*4]*position.z;
        sum += transform[i + 3*4]*position.w;
        transformedPosition[i] = sum;
    }
    //printf("Transformed pos: %f %f %f\n", transformedPosition[0], transformedPosition[1], transformedPosition[2]);

    float4 result = {transformedPosition[0], transformedPosition[1], transformedPosition[2], transformedPosition[3]};
    return result;
}

__kernel void arbitrarySlicing(
        __read_only image3d_t input,
        __write_only image2d_t output,
        __constant float* transform
    ) {

    const int2 position = {get_global_id(0), get_global_id(1)};

    float4 imagePosition = transformPosition(transform, position);
    imagePosition.w = 1;

    int dataType = get_image_channel_data_type(input);
    if(dataType == CLK_FLOAT) {
		float4 value = read_imagef(input, sampler, imagePosition);
		write_imagef(output, position, value);
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
		uint4 value = read_imageui(input, sampler, imagePosition);
		write_imageui(output, position, value);
	} else {
		int4 value = read_imagei(input, sampler, imagePosition);
		write_imagei(output, position, value);
    }
}
