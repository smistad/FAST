__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void clip3D(
        __read_only image3d_t input,
        __global TYPE* output,
        __private float min,
        __private float max
    ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int4 size = {get_global_size(0), get_global_size(1), get_global_size(2), 0};
    const int dataType = get_image_channel_data_type(input);

    if(dataType == CLK_FLOAT) {
        output[pos.x + pos.y*size.x + pos.z*size.x*size.y] = clamp(read_imagef(input, sampler, pos).x, min, max);
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        output[pos.x + pos.y*size.x + pos.z*size.x*size.y] = clamp(read_imageui(input, sampler, pos).x, (uint)min, (uint)max);
    } else {
        output[pos.x + pos.y*size.x + pos.z*size.x*size.y] = clamp(read_imagei(input, sampler, pos).x, (int)min, (int)max);
    }
}
