__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

__kernel void resample3D(
        __read_only image3d_t input,
        __global ushort* output,
        __private float scaleX,
        __private float scaleY,
        __private float scaleZ
) {
    const int4 outputPosition = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int3 size = {get_global_size(0), get_global_size(1), get_global_size(2)};
    float4 inputPosition = {outputPosition.x*(1.0f/scaleX), outputPosition.y*(1.0f/scaleY), outputPosition.z*(1.0f/scaleZ), 0};

    output[outputPosition.x + outputPosition.y*size.x + outputPosition.z*size.x*size.y] = read_imageui(input, sampler, inputPosition).x;
}