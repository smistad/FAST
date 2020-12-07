__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void modifyLabels(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __global uchar* labelChanges,
        __private int count
    ) {

    const int2 pos = {get_global_id(0), get_global_id(1)};
    uchar value = read_imageui(input, sampler, pos).x;
    uchar newValue = value;
    for(int i = 0; i < count-1; ++i) {
        if(value == labelChanges[i*2]) {
            newValue = labelChanges[i*2+1];
        }
    }

    write_imageui(output, pos, newValue);
}