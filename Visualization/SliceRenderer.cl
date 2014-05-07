__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void renderToTexture(
        __read_only image3d_t image,
        __write_only image2d_t texture,
        __private int slice,
        __private float level,
        __private float window
        ) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    // TODO components support
#ifdef TYPE_FLOAT
    float value = read_imagef(image, sampler, (int4)(x,y,slice,0)).x;
#elif TYPE_UINT
    float value = read_imageui(image, sampler, (int4)(x,y,slice,0)).x;
#else
    float value = read_imagei(image, sampler, (int4)(x,y,slice,0)).x;
#endif
    value = (value - level + window*0.5f) / window;
    value = clamp(value, 0.0f, 1.0f);
    //printf("value: %f\n", value);
    write_imagef(texture, (int2)(x,get_global_size(1)-y-1), (float4)(value,value,value,1.0));
}
