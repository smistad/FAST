__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void noneLocalMeans(
		__read_only image3d_t input,
		__write_only image3d_t output
		){
		//Ta inn en pixel, og patchsize
		//Gjøre søket, så ta avg, så legge til den denoisa pixelen i output
}