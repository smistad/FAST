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
