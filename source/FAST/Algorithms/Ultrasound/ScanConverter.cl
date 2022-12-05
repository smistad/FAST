const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_LINEAR;

void cart2pol(float x, float y, float *r, float *th) {
}


__kernel void scanConvert(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float gain,
        __private float dynamicRange,
        __private float newXSpacing,
        __private float newYSpacing,
        __private float startX,
        __private float startY,
        __private float startDepth,
        __private float startAzimuth,
        __private float depthSpacing,
        __private float azimuthSpacing,
        __private int isPolar
    ) {
    int2 pos = {get_global_id(0), get_global_id(1)};
    float x = pos.x*newXSpacing + startX;
    float y = pos.y*newYSpacing + startY;
    // Cart 2 polar
    float r = isPolar == 1 ? sqrt(x*x + y*y) : y;
    float th = isPolar == 1 ? atan2(x, y) : x;

    // Normalize
    r = ((r - startDepth)/depthSpacing)/get_image_height(input);
    th = ((th - startAzimuth)/azimuthSpacing)/get_image_width(input);

    if(r < 0.0f || r > 1.0f || th < 0.0f || th > 1.0f) {
        // Out of bounds
        write_imageui(output, pos, 0);
    } else {
        float dBPixel = read_imagef(input, sampler, (float2)(th, r)).x;
        float img_sc_reject = dBPixel + gain;
        img_sc_reject = (img_sc_reject < -dynamicRange) ? -dynamicRange : img_sc_reject; //Reject everything below dynamic range
        img_sc_reject = (img_sc_reject > 0) ? 0 : img_sc_reject; // Everything above 0 dB should be saturated
        uchar img_gray_scale = round(255*(img_sc_reject+dynamicRange)/dynamicRange);
        write_imageui(output, pos, img_gray_scale);
    }
}

__kernel void scanConvertGrayscale(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float newXSpacing,
        __private float newYSpacing,
        __private float startX,
        __private float startY,
        __private float startDepth,
        __private float startAzimuth,
        __private float depthSpacing,
        __private float azimuthSpacing,
        __private int isPolar
    ) {
    int2 pos = {get_global_id(0), get_global_id(1)};
    float x = pos.x*newXSpacing + startX;
    float y = pos.y*newYSpacing + startY;
    // Cart 2 polar
    float r = isPolar == 1 ? sqrt(x*x + y*y) : y;
    float th = isPolar == 1 ? atan2(x, y) : x;

    // Normalize
    r = ((r - startDepth)/depthSpacing)/get_image_height(input);
    th = ((th - startAzimuth)/azimuthSpacing)/get_image_width(input);

    if(r < 0.0f || r > 1.0f || th < 0.0f || th > 1.0f) {
        // Out of bounds
        write_imageui(output, pos, 0);
    } else {
        write_imageui(output, pos, read_imageui(input, sampler, (float2)(th, r)));
    }
}
