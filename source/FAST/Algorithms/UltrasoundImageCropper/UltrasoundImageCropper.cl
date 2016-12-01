__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;


__kernel void lineSearch(
        __read_only image2d_t image,
        __global uchar* rays
        ) {
    
    const int i = get_global_id(0);
    uchar result = 0;
    const uint width = get_image_width(image);
    const uint height = get_image_height(image);

    if(i < width) {
        for(int y = 0; y < height; ++y) {
            int2 position = {i, y};
            if(read_imageui(image, sampler, position).x > 0) {
                result = 1;
            }
        }
    } else {
        for(int x = 0; x < width; ++x) {
            int2 position = {x, i - width};
            if(read_imageui(image, sampler, position).x > 0) {
                result = 1;
            }
        }
    }

    rays[i] = result;
}
