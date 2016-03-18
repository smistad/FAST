__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;  //border padding //alt: CLK_ADDRESS_MIRRORED_REPEAT - mirrored padding

__kernel void FilteringLocalMemory(
    __read_only image2d_t input,
    __constant float * mask,
    __local float * sharedMem,
    __write_only image2d_t output)
{
    const int2 pos = { get_global_id(0), get_global_id(1) };
    const int globalX = get_global_id(0);
    const int globalY = get_global_id(1);
    const int localX = get_local_id(0);
    const int localY = get_local_id(1);

    // Load top main
    const int localFetchIndex = localX + localY * LOCAL_WIDTH;
    const int2 offsetMain = { -HALF_FILTER_SIZE, -HALF_FILTER_SIZE };
    int dataType = DATA_TYPE;
    if (dataType == CLK_FLOAT) {
        sharedMem[localFetchIndex] = read_imagef(input, sampler, pos + offsetMain).x;
    }
    else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        sharedMem[localFetchIndex] = read_imageui(input, sampler, pos + offsetMain).x;
    }
    else {
        sharedMem[localFetchIndex] = read_imagei(input, sampler, pos + offsetMain).x;
    }
    
    int localIndex;

    // Load top right
    if ( (localX + LOCAL_SIZE_X) < LOCAL_WIDTH){
        const int2 offset_TopRight = { -HALF_FILTER_SIZE + LOCAL_SIZE_X, -HALF_FILTER_SIZE };
        int x_TR = localX + LOCAL_SIZE_X;
        localIndex = x_TR + localY * LOCAL_WIDTH;
        if (dataType == CLK_FLOAT) {
            sharedMem[localIndex] = read_imagef(input, sampler, pos + offset_TopRight).x;
        }
        else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            sharedMem[localIndex] = read_imageui(input, sampler, pos + offset_TopRight).x;
        }
        else {
            sharedMem[localIndex] = read_imagei(input, sampler, pos + offset_TopRight).x;
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE); 

    // Load bottom main  
    if ( (localY + LOCAL_SIZE_Y) < LOCAL_HEIGHT){
        const int2 offset_BottomMain = { -HALF_FILTER_SIZE, -HALF_FILTER_SIZE + LOCAL_SIZE_Y };
        int y_B = localY + LOCAL_SIZE_Y;
        localIndex = localX + y_B * LOCAL_WIDTH;
        if (dataType == CLK_FLOAT) {
            sharedMem[localIndex] = read_imagef(input, sampler, pos + offset_BottomMain).x;
            
        }
        else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            sharedMem[localIndex] = read_imageui(input, sampler, pos + offset_BottomMain).x;
        }
        else {
            sharedMem[localIndex] = read_imagei(input, sampler, pos + offset_BottomMain).x;
        }

        // Load bottom right
        if ( (localX + LOCAL_SIZE_X) < LOCAL_WIDTH){
            const int2 offset_BottomRight = { -HALF_FILTER_SIZE + LOCAL_SIZE_X, -HALF_FILTER_SIZE + LOCAL_SIZE_Y };
            int x_BR = localX + LOCAL_SIZE_X;
            localIndex = x_BR + y_B * LOCAL_WIDTH;
            if (dataType == CLK_FLOAT) {
                sharedMem[localIndex] = read_imagef(input, sampler, pos + offset_BottomRight).x;
            }
            else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
                sharedMem[localIndex] = read_imageui(input, sampler, pos + offset_BottomRight).x;
            }
            else {
                sharedMem[localIndex] = read_imagei(input, sampler, pos + offset_BottomRight).x;
            }
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    float sum = 0.0f;
    int index = localX + localY * LOCAL_WIDTH;
    int ySizeAdd = 0;
    for (int y = -HALF_FILTER_SIZE; y <= HALF_FILTER_SIZE; y++){
        for (int x = -HALF_FILTER_SIZE; x <= HALF_FILTER_SIZE; x++){
            sum += sharedMem[index++] * mask[ySizeAdd++];
        }
        index += LOCAL_WIDTH - FILTER_SIZE;
    }

    sum = fabs(sum);
    if (IMAGE_WIDTH > globalX && IMAGE_HEIGHT > globalY){
        int outputDataType = DATA_TYPE;
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
}