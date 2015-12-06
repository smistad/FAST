__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void convertToHU(
	__read_only image3d_t input,
	__write_only image3d_t output
	) {
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	
	int value = read_imageui(input, sampler, pos).x;
	value -= 1024;
	write_imagei(output, pos, value);
}