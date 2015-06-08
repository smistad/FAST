__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void laplacianOfGaussian(
        __read_only image2d_t input,
        __constant float * mask,
        __write_only image2d_t output,
        __private unsigned char maskSize
        ) {

    const int2 pos = {get_global_id(0), get_global_id(1)};
    const unsigned char halfSize = (maskSize-1)/2;

    float sum = 0.0f;
    for(int x = -halfSize; x <= halfSize; x++) {
    for(int y = -halfSize; y <= halfSize; y++) {
        const int2 offset = {x,y};
#ifdef TYPE_FLOAT
        sum += mask[x+halfSize+(y+halfSize)*maskSize]*read_imagef(input, sampler, pos+offset).x;
#elif TYPE_UINT
        sum += mask[x+halfSize+(y+halfSize)*maskSize]*read_imageui(input, sampler, pos+offset).x;
#else
        sum += mask[x+halfSize+(y+halfSize)*maskSize]*read_imagei(input, sampler, pos+offset).x;
#endif
    }}

    write_imagef(output, pos, sum);
}
