//__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;  //border padding
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_NEAREST; // mirrored padding
#pragma OPENCL EXTENSION cl_intel_printf : enable


__kernel void SpatialFilter(
        __read_only image2d_t input,
        __constant float * mask,
        __write_only image2d_t output,
        __private unsigned char maskSize
        ) {

    const int2 pos = {get_global_id(0), get_global_id(1)};
    const unsigned char halfSize = (maskSize-1)/2;

    float sum = 0.0f;
    int dataType = get_image_channel_data_type(input);
    for(int x = -halfSize; x <= halfSize; x++) {
    for(int y = -halfSize; y <= halfSize; y++) {
        const int2 offset = {x,y};
        if(dataType == CLK_FLOAT) {
            sum += mask[x+halfSize+(y+halfSize)*maskSize]*read_imagef(input, sampler, pos+offset).x;
        } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            sum += mask[x+halfSize+(y+halfSize)*maskSize]*read_imageui(input, sampler, pos+offset).x;
        } else {
            sum += mask[x+halfSize+(y+halfSize)*maskSize]*read_imagei(input, sampler, pos+offset).x;
        }
    }}

    //sum += STATIC_ADD;
    sum = fabs(sum);

    int outputDataType = get_image_channel_data_type(output);
    if(outputDataType == CLK_FLOAT) {
        write_imagef(output, pos, sum);
    } else if(outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(output, pos, round(sum));
    } else {
        write_imagei(output, pos, round(sum));
    }
}

__kernel void OneDirPass(
    __read_only image2d_t input,
    __constant float * mask,
    __write_only image2d_t output)
{
    const int2 pos = { get_global_id(0), get_global_id(1) };
    //define halfSize in params

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
        /*
        const int2 offsetFirst = { x, -HALF_FILTER_SIZE };
        if (dataType == CLK_FLOAT) {
            sum = read_imagef(input, sampler, pos + offsetFirst).x;
        }
        else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            sum = read_imageui(input, sampler, pos + offsetFirst).x;
        }
        else {
            sum = read_imagei(input, sampler, pos + offsetFirst).x;
        }
        */
        //sum += STATIC_ADD;
        //sum = fabs(sum);
    }

    //sum += STATIC_ADD/2.0;
    //sum = fabs(sum);

    /*
    if (sum > 1.0){
        sum = 1.0;
    }
    else if (sum < 0.0){
        sum = 0.0;
    }*/

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

__kernel void FilteringLocalMemory(
    __read_only image2d_t input,
    __constant float * mask,
    __local float * sharedMem,
    __write_only image2d_t output)
{
    // image size x and y, parameters? definable?
    const int2 pos = { get_global_id(0), get_global_id(1) };
    const int globalX = get_global_id(0);
    const int globalY = get_global_id(1);
    //const int2 localPos = { get_local_id(0), get_local_id(1) };
    const int localX = get_local_id(0);
    const int localY = get_local_id(1);
    const int localSizeX = get_local_size(0);
    const int localSizeY = get_local_size(1);

    //Load to local
    //const int localIndex = globalX + globalY * localSizeX;//(32 + LOCAL_MEM_PAD);
    //const int localIndex = (HALF_FILTER_SIZE + globalX) + ((HALF_FILTER_SIZE + globalY) * localSizeX);
    //const int localIndex = localX + HALF_FILTER_SIZE + (localY + HALF_FILTER_SIZE)*(32 + LOCAL_MEM_PAD);
    const int2 offset = { HALF_FILTER_SIZE, HALF_FILTER_SIZE };
    const int2 realPos = pos - offset;
    //int sharedX = sizeof(sharedMem);
    //printf("SharedX: %d\n", sharedX);
    return;
    int dataType = get_image_channel_data_type(input);
    if (dataType == CLK_FLOAT) {
        sharedMem[localX][localY] = read_imagef(input, sampler, realPos).x;
    }
    else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        sharedMem[localX][localY] = read_imageui(input, sampler, realPos).x;
    }
    else {
        sharedMem[localX][localY] = read_imagei(input, sampler, realPos).x;
    }
    bool safe = false;
    int imSizeX = get_image_width(input);
    int imSizeY = get_image_height(input);
    if (localX == 0 && localY == 0){
        // calculate safe?
        if (globalX > HALF_FILTER_SIZE && (globalX + localSizeX) < (HALF_FILTER_SIZE + imSizeX)){
            if (globalY > HALF_FILTER_SIZE && (globalY + localSizeY) < (HALF_FILTER_SIZE + imSizeY)){
                safe = true; // no pixels in this local neighbourhood is outside real image
            }
        }
        // load remaining pixels?
    }
    barrier(CLK_LOCAL_MEM_FENCE); // let all be loaded // good placement?
    //define halfSize in params
    if (safe || (globalX > HALF_FILTER_SIZE && globalX < (HALF_FILTER_SIZE + imSizeX) 
            && globalY > HALF_FILTER_SIZE && globalX < (HALF_FILTER_SIZE + imSizeY) ) ) {
        // NAIVE
        float sum = 0.0f;
        int index = globalX + globalY*localSizeX;
        int ySizeAdd;
        for (int y = -HALF_FILTER_SIZE; y <= HALF_FILTER_SIZE; y++){
            ySizeAdd = (y + HALF_FILTER_SIZE)*FILTER_SIZE + HALF_FILTER_SIZE;
            for (int x = -HALF_FILTER_SIZE; x <= HALF_FILTER_SIZE; x++){
                //int offset = { x, y };
                //index = (HALF_FILTER_SIZE + globalX + x) + ((HALF_FILTER_SIZE + globalY + y) * localSizeX);
                sum += sharedMem[x][y] * mask[x + ySizeAdd];// halfSize + (y + halfSize)*maskSize];
                //index++;
            }
            //index += localSizeX
            //index += LOCAL_MEM_PAD; //?
        }

        //sharedMem[localIndex] = sum;
        int outputDataType = get_image_channel_data_type(output);
        if (outputDataType == CLK_FLOAT) {
            write_imagef(output, realPos, sum);
        }
        else if (outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
            write_imageui(output, realPos, round(sum));
        }
        else {
            write_imagei(output, realPos, round(sum));
        }
        
    }
    return;
    /*
    else {
    // ----- PASS 0 ----- //
    float sum = 0.0f;
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
    barrier(CLK_LOCAL_MEM_FENCE); // let all in pass 1 finish
    // ----- PASS 1 ----- //
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
    
    const int2 offsetFirst = { x, -HALF_FILTER_SIZE };
    if (dataType == CLK_FLOAT) {
    sum = read_imagef(input, sampler, pos + offsetFirst).x;
    }
    else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
    sum = read_imageui(input, sampler, pos + offsetFirst).x;
    }
    else {
    sum = read_imagei(input, sampler, pos + offsetFirst).x;
    }
    */
    //sum += STATIC_ADD;
    //sum = fabs(sum);
    //}

    //sum += STATIC_ADD/2.0;
    //sum = fabs(sum);

    /*
    if (sum > 1.0){
    sum = 1.0;
    }
    else if (sum < 0.0){
    sum = 0.0;
    }*/

    /*
    int outputDataType = get_image_channel_data_type(output);
    if (outputDataType == CLK_FLOAT) {
        write_imagef(output, pos, sum);
    }
    else if (outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(output, pos, round(sum));
    }
    else {
        write_imagei(output, pos, round(sum));
    }*/
}
