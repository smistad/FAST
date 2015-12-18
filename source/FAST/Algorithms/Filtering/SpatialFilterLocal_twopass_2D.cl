__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;  //border padding
//__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_NEAREST; // mirrored padding

__kernel void FilteringLocalMemory_twopass(
    __read_only image2d_t input,
    __constant float * maskX,
    __constant float * maskY,
    __local float * sharedMem,
    __write_only image2d_t output)
{
    // TODO check if border pic by using get_group_id()?
    // image size x and y, parameters? definable?
    const int2 pos = { get_global_id(0), get_global_id(1) };
    const int globalX = get_global_id(0);
    const int globalY = get_global_id(1);
    const int localX = get_local_id(0);
    const int localY = get_local_id(1);
    //const int localSizeX = get_local_size(0);
    //const int localSizeY = get_local_size(1);
    //const int localSizeTot = LOCAL_SIZE_X * LOCAL_SIZE_Y;
    //const int localWidth = LOCAL_SIZE_X + 2 * HALF_FILTER_SIZE + 1;
    //const int localHeight = LOCAL_SIZE_Y + 2 * HALF_FILTER_SIZE;
    //const int localTot = localWidth * localHeight;
    // todo define these as build params or sth!

    // Load top main
    const int localFetchIndex = localX + localY * LOCAL_WIDTH;//LOCAL_SIZE_X; // ja continuelig
    //const int localIndex = localX + localY * LOCAL_SIZE_X;
    const int2 offsetMain = { -HALF_FILTER_SIZE, -HALF_FILTER_SIZE };
    int dataType = DATA_TYPE; // get_image_channel_data_type(input);
    if (dataType == CLK_FLOAT) {
        sharedMem[localFetchIndex] = read_imagef(input, sampler, pos + offsetMain).x;
    }
    else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        sharedMem[localFetchIndex] = read_imageui(input, sampler, pos + offsetMain).x;
    }
    else {
        sharedMem[localFetchIndex] = read_imagei(input, sampler, pos + offsetMain).x;
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    //int x;
    int localIndex;
    //int2 posBase = { (globalX - localX - HALF_FILTER_SIZE), (globalY - localY - HALF_FILTER_SIZE) };
    // Load top right
    if ((localX + LOCAL_SIZE_X) < LOCAL_WIDTH){
        const int2 offset_TopRight = { -HALF_FILTER_SIZE + LOCAL_SIZE_X, -HALF_FILTER_SIZE };
        int x_TR = localX + LOCAL_SIZE_X;
        localIndex = x_TR + localY * LOCAL_WIDTH;
        //int dataType = DATA_TYPE; // get_image_channel_data_type(input);
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
    // TODO change read for all localY + LOCAL_SIZE_Y < LOCAL_HEIGHT
    // Load bottom main  
    if ((localY + LOCAL_SIZE_Y) < LOCAL_HEIGHT){
        const int2 offset_BottomMain = { -HALF_FILTER_SIZE, -HALF_FILTER_SIZE + LOCAL_SIZE_Y };
        int y_B = localY + LOCAL_SIZE_Y;
        localIndex = localX + y_B * LOCAL_WIDTH;
        //int dataType = DATA_TYPE; // get_image_channel_data_type(input);
        if (dataType == CLK_FLOAT) {
            //if (localY == 0) sharedMem[localIndex] = 1.0f;
            //else sharedMem[localIndex] = 0.2f;
            sharedMem[localIndex] = read_imagef(input, sampler, pos + offset_BottomMain).x;

        }
        else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            sharedMem[localIndex] = read_imageui(input, sampler, pos + offset_BottomMain).x;
        }
        else {
            sharedMem[localIndex] = read_imagei(input, sampler, pos + offset_BottomMain).x;
        }

        // Load bottom right
        if ((localX + LOCAL_SIZE_X) < LOCAL_WIDTH){
            const int2 offset_BottomRight = { -HALF_FILTER_SIZE + LOCAL_SIZE_X, -HALF_FILTER_SIZE + LOCAL_SIZE_Y };
            int x_BR = localX + LOCAL_SIZE_X;
            localIndex = x_BR + y_B * LOCAL_WIDTH;
            //int dataType = DATA_TYPE; // get_image_channel_data_type(input);
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
    barrier(CLK_LOCAL_MEM_FENCE); // BLOCK AFTER LOADING IN
    
    int maskBaseToCentreOffset = HALF_FILTER_SIZE + (HALF_FILTER_SIZE * LOCAL_WIDTH);
    // HORIZONTAL PASS - group 1
    float sum = 0.0f;
    int g1_x = localX;// +HALF_FILTER_SIZE;
    int g1_y = localY;// -HALF_FILTER_SIZE;
    int baseIndex_g1 = g1_x + (g1_y * LOCAL_WIDTH);// +HALF_FILTER_SIZE;
    int index = baseIndex_g1;// -HALF_FILTER_SIZE;
    int maskIndex = 0;
    //for (int x_it = -HALF_FILTER_SIZE; x_it <= HALF_FILTER_SIZE; x_it++){
    for (int maskIndex1X = 0; maskIndex1X <= FILTER_SIZE; maskIndex1X++){
        sum += sharedMem[index++] * maskX[maskIndex1X];// ++];
    }
    barrier(CLK_LOCAL_MEM_FENCE); // BLOCK FOR SAVING sum HORIZONTAL group 1
    int centreIndex_g1 = baseIndex_g1 + HALF_FILTER_SIZE;
    sharedMem[centreIndex_g1] = sum;
    //sharedMem[5 + 5 * LOCAL_WIDTH] = 2.0f;
    barrier(CLK_LOCAL_MEM_FENCE);
    // HORIZONTAL PASS - group 2
    int g2_y = g1_y + LOCAL_SIZE_Y;
    if (g2_y < LOCAL_HEIGHT){
        sum = 0.0f;
        int g2_x = g1_x; // localX - HALF_FILTER_SIZE;
        int baseIndex_g2 = g2_x + (g2_y * LOCAL_WIDTH);// +HALF_FILTER_SIZE;
        index = baseIndex_g2;
        //maskIndex = 0;
        //for (int x_it = -HALF_FILTER_SIZE; x_it <= HALF_FILTER_SIZE; x_it++){
        for (int maskIndex2X = 0; maskIndex2X <= FILTER_SIZE; maskIndex2X++){
            sum += sharedMem[index++] * maskX[maskIndex2X];// ++];
        }
        //barrier(CLK_LOCAL_MEM_FENCE); // BLOCK FOR SAVING sum HORIZONTAL group 2
        int centreIndex_g2 = baseIndex_g2 + HALF_FILTER_SIZE;
        sharedMem[centreIndex_g2] = sum;
    }

    barrier(CLK_LOCAL_MEM_FENCE); // BLOCK FOR HORIZONTAL FINISHED
    
    // VERTICAL PASS - group 1
    sum = 0.0f;
    //float outValue = 0.0f;
    //g1_x = localX - HALF_FILTER_SIZE;
    int g3_x = localX + HALF_FILTER_SIZE;
    int g3_y = localY;// +HALF_FILTER_SIZE;
    int baseIndex_vert = g3_x + g3_y * LOCAL_WIDTH;// +maskBaseToCentreOffset;
    index = baseIndex_vert;// -maskBaseToCentreOffset;
    //maskIndex = 0;
    barrier(CLK_LOCAL_MEM_FENCE);
    //for (int y_it = -HALF_FILTER_SIZE; y_it <= HALF_FILTER_SIZE; y_it++){
    for (int maskIndexY = 0; maskIndexY <= FILTER_SIZE; maskIndexY++){
        sum += sharedMem[index] * maskY[maskIndexY];// maskIndex++];
        index += LOCAL_WIDTH; // no -filtersize?
    }
    //barrier(CLK_LOCAL_MEM_FENCE); // BLOCK FOR SAVING Vertical
    //int ceQntreIndex_vert = baseIndex_vert + (HALF_FILTER_SIZE*LOCAL_WIDTH);
    //sharedMem[g3_x + ((g3_y + HALF_FILTER_SIZE)*LOCAL_WIDTH)] = sum;
    /**/
    //barrier(CLK_LOCAL_MEM_FENCE); // BLOCK FOR VERTICAL FINISHED
    
    //float sum = 0.7f;
    //int writeIndex = baseIndex_vert;
    int writeIndex = (localX+HALF_FILTER_SIZE) + ((localY+HALF_FILTER_SIZE) * LOCAL_WIDTH);
    //float outValue = sum;// sharedMem[writeIndex];
    //sum = sharedMem[writeIndex];
    //if (localX == 0 && localY == 0) sum = 1.0f;
    //if (localX == LOCAL_SIZE_X-1 || localY == LOCAL_SIZE_Y-1) sum = 0.3f;
    //int imSizeX = get_image_width(input);
    //int imSizeY = get_image_height(input);

    barrier(CLK_LOCAL_MEM_FENCE);
    if (IMAGE_WIDTH > globalX && IMAGE_HEIGHT > globalY){
        int outputDataType = DATA_TYPE;// get_image_channel_data_type(output);
        if (outputDataType == CLK_FLOAT) {
            write_imagef(output, pos, sum);// sharedMem[writeIndex]);
        }
        else if (outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
            write_imageui(output, pos, round(sum));//sharedMem[writeIndex]));
        }
        else {
            write_imagei(output, pos, round(sum));//sharedMem[writeIndex]));
        }
    }
}

