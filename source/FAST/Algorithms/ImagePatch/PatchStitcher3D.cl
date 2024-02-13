__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

#ifdef fast_3d_image_writes
__kernel void applyPatch2Dto3D(
        __read_only image2d_t patch,
        __write_only image3d_t image,
        __private int startX,
        __private int startY,
        __private int startZ
    ) {
    const int4 pos = {get_global_id(0) + startX, get_global_id(1) + startY, get_global_id(2) + startZ, 0};
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT) {
		write_imagef(image, pos, read_imagef(patch, sampler, pos.xy - (int2)(startX, startY)));
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
		write_imageui(image, pos, read_imageui(patch, sampler, pos.xy - (int2)(startX, startY)));
	} else {
		write_imagei(image, pos, read_imagei(patch, sampler, pos.xy - (int2)(startX, startY)));
    }
}
__kernel void applyPatch3D(
        __read_only image3d_t patch,
        __write_only image3d_t image,
        __private int startX,
        __private int startY,
        __private int startZ
    ) {
    const int4 pos = {get_global_id(0) + startX, get_global_id(1) + startY, get_global_id(2) + startZ, 0};
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT) {
		write_imagef(image, pos, read_imagef(patch, sampler, pos - (int4)(startX, startY, startZ, 0)));
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
		write_imageui(image, pos, read_imageui(patch, sampler, pos - (int4)(startX, startY, startZ, 0)));
	} else {
		write_imagei(image, pos, read_imagei(patch, sampler, pos - (int4)(startX, startY, startZ, 0)));
    }
}
#else
__kernel void applyPatch2Dto3D(
        __read_only image2d_t patch,
        __global TYPE* image,
        __private int startX,
        __private int startY,
        __private int startZ,
        __private int width,
        __private int height,
        __private int channels
    ) {
    const int4 pos = {get_global_id(0) + startX, get_global_id(1) + startY, get_global_id(2) + startZ, 0};
    int dataType = get_image_channel_data_type(patch);
    float4 value;
    if(dataType == CLK_FLOAT) {
		value = read_imagef(patch, sampler, pos.xy - (int2)(startX, startY));
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
		value = convert_float4(read_imageui(patch, sampler, pos.xy - (int2)(startX, startY)));
	} else {
		value = convert_float4(read_imagei(patch, sampler, pos.xy - (int2)(startX, startY)));
    }
    image[(pos.x + pos.y*width + pos.z*width*height)*channels] = value.x;
    if(channels > 1)
        image[(pos.x + pos.y*width + pos.z*width*height)*channels + 1] = value.y;
    if(channels > 2)
        image[(pos.x + pos.y*width + pos.z*width*height)*channels + 2] = value.z;
    if(channels > 3)
        image[(pos.x + pos.y*width + pos.z*width*height)*channels + 3] = value.w;
}
__kernel void applyPatch3D(
        __read_only image3d_t patch,
        __global TYPE* image,
        __private int startX,
        __private int startY,
        __private int startZ,
        __private int width,
        __private int height,
        __private int channels
    ) {
    const int4 pos = {get_global_id(0) + startX, get_global_id(1) + startY, get_global_id(2) + startZ, 0};
    int dataType = get_image_channel_data_type(patch);
    float4 value;
    if(dataType == CLK_FLOAT) {
		value = read_imagef(patch, sampler, pos - (int4)(startX, startY, startZ, 0));
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
		value = convert_float4(read_imageui(patch, sampler, pos - (int4)(startX, startY, startZ, 0)));
	} else {
		value = convert_float4(read_imagei(patch, sampler, pos - (int4)(startX, startY, startZ, 0)));
    }
    image[(pos.x + pos.y*width + pos.z*width*height)*channels] = value.x;
    if(channels > 1)
        image[(pos.x + pos.y*width + pos.z*width*height)*channels + 1] = value.y;
    if(channels > 2)
        image[(pos.x + pos.y*width + pos.z*width*height)*channels + 2] = value.z;
    if(channels > 3)
        image[(pos.x + pos.y*width + pos.z*width*height)*channels + 3] = value.w;
}
#endif
