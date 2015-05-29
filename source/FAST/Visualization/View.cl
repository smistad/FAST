
__kernel void initializePBO(
        __global float* PBO
        ) {
    const float4 value = {1.0, 1.0, 1.0, 1.0};
    vstore4(value, get_global_id(0), PBO);
}