const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;

__kernel void transpose3D(
    __read_only image3d_t input,
    __global TYPE* output,
    __private int axes1,
    __private int axes2,
    __private int axes3,
    __private int channels
    ) {

    int pos[3] = {get_global_id(0), get_global_id(1), get_global_id(2)};
    int4 posInput = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    int size[3] = {get_image_width(input), get_image_height(input), get_image_depth(input)};

    int dataType = get_image_channel_data_type(input);
    if(dataType == CLK_FLOAT) {
        float4 value = read_imagef(input, sampler, posInput);
        output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels] = value.x;
        if(channels > 1)
            output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels + 1] = value.y;
        if(channels > 2)
            output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels + 2] = value.z;
        if(channels > 3)
            output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels + 3] = value.w;
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        uint4 value = read_imageui(input, sampler, posInput);
        output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels] = value.x;
        if(channels > 1)
            output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels + 1] = value.y;
        if(channels > 2)
            output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels + 2] = value.z;
        if(channels > 3)
            output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels + 3] = value.w;
    } else {
        int4 value = read_imagei(input, sampler, posInput);
        output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels] = value.x;
        if(channels > 1)
            output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels + 1] = value.y;
        if(channels > 2)
            output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels + 2] = value.z;
        if(channels > 3)
            output[(pos[axes1] + pos[axes2]*size[axes1] + pos[axes3]*size[axes1]*size[axes2])*channels + 3] = value.w;
    }
}