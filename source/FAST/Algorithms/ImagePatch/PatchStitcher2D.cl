__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void applyPatch2D(
        __read_only image2d_t patch,
        __write_only image2d_t image,
        __private int startX,
        __private int startY
    ) {
    const int2 pos = {get_global_id(0) + startX, get_global_id(1) + startY};
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT) {
		write_imagef(image, pos, read_imagef(patch, sampler, pos - (int2)(startX, startY)));
	} else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
		write_imageui(image, pos, read_imageui(patch, sampler, pos - (int2)(startX, startY)));
	} else {
		write_imagei(image, pos, read_imagei(patch, sampler, pos - (int2)(startX, startY)));
    }
}
