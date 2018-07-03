__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;


__kernel void lineSearch(
        __read_only image2d_t image,
        __global uint* rays
        ) {
    
    const int i = get_global_id(0);
    uint result = 0;
    const int width = get_image_width(image);
    const int height = get_image_height(image);

    if(i < width) {
        for(int y = height/6; y < height*5/6; ++y) {
            int2 position = {i, y};
            if(read_imageui(image, sampler, position).x > 0) {
                result += 1;
            }
        }
    } else {
        for(int x = width/6; x < width*5/6; ++x) {
            int2 position = {x, i - width};
            if(read_imageui(image, sampler, position).x > 0) {
                result += 1;
            }
        }
    }

    rays[i] = result;
}
