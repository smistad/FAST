//__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void zeroInitBuffer(
    __global unsigned int* volume
    ){
    const int x = get_global_id(0);
    volume[x] = 0;
}