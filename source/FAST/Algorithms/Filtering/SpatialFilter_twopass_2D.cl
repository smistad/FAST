__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;  //border padding // CLK_ADDRESS_MIRRORED_REPEAT - mirrored padding

__kernel void OneDirPass(
    __read_only image2d_t input,
    __constant float * mask,
    __write_only image2d_t output)
{
    const int2 pos = { get_global_id(0), get_global_id(1) };

    float sum = 0.0f;
    int dataType = get_image_channel_data_type(input);
    
    if (PASS_DIRECTION == 0){
        int y = 0;
        for (int x = -HALF_FILTER_SIZE; x <= HALF_FILTER_SIZE; x++){
            const int2 offset = { x, y };
            if (dataType == CLK_FLOAT) {
                sum += mask[x + HALF_FILTER_SIZE] * read_imagef(input, sampler, pos + offset).x;
            }
            else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
                sum += mask[x + HALF_FILTER_SIZE] * read_imageui(input, sampler, pos + offset).x;
            }
            else {
                sum += mask[x + HALF_FILTER_SIZE] * read_imagei(input, sampler, pos + offset).x;
            }
        }
    }
    else if (PASS_DIRECTION == 1){
        int x = 0;
        for (int y = -HALF_FILTER_SIZE; y <= HALF_FILTER_SIZE; y++){
            const int2 offset = { x, y };
            if (dataType == CLK_FLOAT) {
                sum += mask[y + HALF_FILTER_SIZE] * read_imagef(input, sampler, pos + offset).x;
            }
            else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
                sum += mask[y + HALF_FILTER_SIZE] * read_imageui(input, sampler, pos + offset).x;
            }
            else {
                sum += mask[y + HALF_FILTER_SIZE] * read_imagei(input, sampler, pos + offset).x;
            }
        }
        sum = fabs(sum);
    }

    int outputDataType = get_image_channel_data_type(output);
    if (outputDataType == CLK_FLOAT) {
        write_imagef(output, pos, sum);
    }
    else if (outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(output, pos, round(sum));
    }
    else {
        write_imagei(output, pos, round(sum));
    }
}