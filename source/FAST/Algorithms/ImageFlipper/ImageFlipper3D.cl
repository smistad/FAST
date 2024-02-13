const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;

__kernel void flip3D(
    __read_only image3d_t input,
    __global TYPE* output,
    __private char flipHorizontal,
    __private char flipVertical,
    __private char flipDepth,
    __private int channels
    ) {

    int4 posInput = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    int3 targetPos = posInput.xyz;
    int3 size = {get_image_width(input), get_image_height(input), get_image_depth(input)};
    if(flipHorizontal == 1)
        targetPos.x = size.x - posInput.x - 1;
    if(flipVertical == 1)
        targetPos.y = size.y - posInput.y - 1;
    if(flipDepth == 1)
        targetPos.z = size.z - posInput.z - 1;

    int dataType = get_image_channel_data_type(input);
    if(dataType == CLK_FLOAT) {
        float4 value = read_imagef(input, sampler, posInput);
        output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels] = value.x;
        if(channels > 1)
		output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels+1] = value.y;
        if(channels > 2)
		output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels+2] = value.z;
        if(channels > 3)
		output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels+3] = value.w;
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        uint4 value = read_imageui(input, sampler, posInput);
        output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels] = value.x;
        if(channels > 1)
		output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels+1] = value.y;
        if(channels > 2)
		output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels+2] = value.z;
        if(channels > 3)
		output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels+3] = value.w;
    } else {
        int4 value = read_imagei(input, sampler, posInput);
        output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels] = value.x;
        if(channels > 1)
		output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels+1] = value.y;
        if(channels > 2)
		output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels+2] = value.z;
        if(channels > 3)
		output[(targetPos.x + targetPos.y*size.x + targetPos.z*size.x*size.y)*channels+3] = value.w;
    }
}
