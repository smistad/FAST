
__kernel void doubleFilter(
        __global TYPE * input,
        __global TYPE * output
        ) {
    output[get_global_id(0)] = input[get_global_id(0)]*2;
}
