__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void renderToTexture(
        __read_only image3d_t image,
        __write_only image2d_t texture,
        __private int slice,
        __private float level,
        __private float window,
        __private int slicePlane
        ) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    // TODO make sure that these positions are correct
    int4 pos;
    if(slicePlane == 0) {
        pos = (int4)(slice,y,x,0);
    } else if(slicePlane == 1) {
        pos = (int4)(x,slice,y,0);
    } else {
        pos = (int4)(x,y,slice,0);
    }

    // TODO components support
#ifdef TYPE_FLOAT
    float value = read_imagef(image, sampler, pos).x;
#elif TYPE_UINT
    float value = read_imageui(image, sampler, pos).x;
#else
    float value = read_imagei(image, sampler, pos).x;
#endif
    value = (value - level + window/2) / window;
    value = clamp(value, 0.0f, 1.0f);
    //printf("value: %f\n", value);
    write_imagef(texture, (int2)(x,get_global_size(1)-y-1), (float4)(value,value,value,1.0));
}
