__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

float getIntensity(__read_only image3d_t image, int4 pos) {
    float value;
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT) {
        value = read_imagef(image, sampler, pos).x;
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        value = read_imageui(image, sampler, pos).x;
    } else {
        value = read_imagei(image, sampler, pos).x;
    }
    return value;
}

__kernel void thresholding(
        __read_only image3d_t image,
        __write_only image3d_t segmentation,
        __private float lowerThreshold,
        __private float upperThreshold
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    
    const float value = getIntensity(image, pos);
   
    uchar writeValue = 0;
    if(value >= lowerThreshold && value <= upperThreshold) {
        writeValue = 1;
    }
    write_imageui(segmentation, pos, writeValue);
}
__kernel void thresholdingWithOnlyLower(
        __read_only image3d_t image,
        __write_only image3d_t segmentation,
        __private float lowerThreshold
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    
    const float value = getIntensity(image, pos);

    uchar writeValue = 0;
    if(value >= lowerThreshold) {
        writeValue = 1;
    }
    write_imageui(segmentation, pos, writeValue);
}
__kernel void thresholdingWithOnlyUpper(
        __read_only image3d_t image,
        __write_only image3d_t segmentation,
        __private float upperThreshold
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    
    const float value = getIntensity(image, pos);

    uchar writeValue = 0;
    if(value <= upperThreshold) {
        writeValue = 1;
    }
    write_imageui(segmentation, pos, writeValue);
}