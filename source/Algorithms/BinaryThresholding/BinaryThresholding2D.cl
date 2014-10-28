
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void thresholding(
        __read_only image2d_t image,
        __write_only image2d_t segmentation,
        __private float lowerThreshold,
        __private float upperThreshold
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    
    const float value = read_imagef(image, sampler, pos).x;
    float writeValue = 0;
    if(value >= lowerThreshold && value <= lowerThreshold) {
        writeValue = 1;
    }
    write_imageui(segmentation, pos, writeValue);
}
__kernel void thresholdingWithOnlyLower(
        __read_only image2d_t image,
        __write_only image2d_t segmentation,
        __private float lowerThreshold
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    
    const float value = read_imagef(image, sampler, pos).x;
    float writeValue = 0;
    if(value >= lowerThreshold) {
        writeValue = 1;
    }
    write_imageui(segmentation, pos, writeValue);
}
__kernel void thresholdingWithOnlyUpper(
        __read_only image2d_t image,
        __write_only image2d_t segmentation,
        __private float upperThreshold
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    
    const float value = read_imagef(image, sampler, pos).x;
    float writeValue = 0;
    if(value <= upperThreshold) {
        writeValue = 1;
    }
    write_imageui(segmentation, pos, writeValue);
}