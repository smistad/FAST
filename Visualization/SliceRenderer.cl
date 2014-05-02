__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void renderToTexture(
        __read_only image3d_t image,
        __write_only image2d_t texture,
        __private int slice
        ) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    float value = (float)read_imageui(image, sampler, (int4)(x,y,slice,0)).x/255;
    //printf("value: %f\n", value);
    write_imagef(texture, (int2)(x,get_global_size(1)-y-1), (float4)(value,value,value,1.0));
}
