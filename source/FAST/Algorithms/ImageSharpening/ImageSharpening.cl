__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void sharpen(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float stdDev,
        __private float gain
        ) {

    const int2 pos = {get_global_id(0), get_global_id(1)};

    float4 sum = {0.0f, 0.0f, 0.0f, 0.0f};
    float gaussianSum = 0.0f;
    const int dataType = get_image_channel_data_type(input);
    for(int x = -HALF_SIZE; x <= HALF_SIZE; ++x) {
    for(int y = -HALF_SIZE; y <= HALF_SIZE; ++y) {
        const int2 offset = {x,y};
        const float gaussian =  exp(-(float)(x*x+y*y)/(2.0f*stdDev*stdDev));
        gaussianSum += gaussian;
        if(dataType == CLK_FLOAT) {
            sum += gaussian*read_imagef(input, sampler, pos+offset);
        } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            sum += gaussian*convert_float4(read_imageui(input, sampler, pos+offset));
        } else {
            sum += gaussian*convert_float4(read_imagei(input, sampler, pos+offset));
        }
    }}
    float4 pixel;
    if(dataType == CLK_FLOAT) {
        pixel = read_imagef(input, sampler, pos);
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        pixel = convert_float4(read_imageui(input, sampler, pos));
    } else {
        pixel = convert_float4(read_imagei(input, sampler, pos));
    }

    float4 result = pixel + gain*(pixel - sum/gaussianSum);

    const int outputDataType = get_image_channel_data_type(output);
    if(outputDataType == CLK_FLOAT) {
        write_imagef(output, pos, result);
    } else if(outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(output, pos, convert_uint4(round(result)));
    } else {
        write_imagei(output, pos, convert_int4(round(result)));
    }
}
