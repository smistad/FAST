
__kernel void initializePBO(
        __global float* PBO,
        __private float red,
        __private float green,
        __private float blue
        ) {
    const float4 value = {red, green, blue, 1.0};
    vstore4(value, get_global_id(0), PBO);
}