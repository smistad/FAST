__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

float getPixelAsFloat(__read_only image2d_t image, int2 pos) {
    float value;
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT) {
        value = read_imagef(image, sampler, pos).x;
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        value = read_imageui(image, sampler, pos).x;
    } else {
        value = read_imagei(image, sampler, pos).x;
    }
    return value;
}

float calculateMeanIntensity(__read_only image2d_t frame, int2 pos) {
    float mean = 0.0f;
    for(int y = pos.y - BLOCK_SIZE; y <= pos.y + BLOCK_SIZE; ++y) {
        for(int x = pos.x - BLOCK_SIZE; x <= pos.x + BLOCK_SIZE; ++x) {
            mean += getPixelAsFloat(frame, (int2)(x,y));
        }
    }

    return mean/((BLOCK_SIZE*2+1)*(BLOCK_SIZE*2+1));
}


inline float2 findSubpixelMovement(const float2 movement, const float b[GRID_SIZE][GRID_SIZE]) {
    const int index_x = (int)movement.x + SEARCH_SIZE+1;
    const int index_y = (int)movement.y + SEARCH_SIZE+1;
    // Subpixel parabolic fitting
    const float A =
            (b[index_x-1][index_y-1] - 2 * b[index_x][index_y-1] + b[index_x+1][index_y-1] + b[index_x-1][index_y] - 2 * b[index_x][index_y] + b[index_x+1][index_y] + b[index_x-1][index_y+1] - 2 * b[index_x][index_y+1] +
             b[index_x+1][index_y+1]) / 6.0;
    const float B = (b[index_x-1][index_y-1] - b[index_x+1][index_y-1] - b[index_x-1][index_y+1] + b[index_x+1][index_y+1]) / 4.0;
    const float C =
            (b[index_x-1][index_y-1] + b[index_x][index_y-1] + b[index_x+1][index_y-1] - 2 * b[index_x-1][index_y] - 2 * b[index_x][index_y] - 2 * b[index_x+1][index_y] + b[index_x-1][index_y+1] + b[index_x][index_y+1] +
             b[index_x+1][index_y+1]) / 6.0;
    const float D = (-b[index_x-1][index_y-1] + b[index_x+1][index_y-1] - b[index_x-1][index_y] + b[index_x+1][index_y] - b[index_x-1][index_y+1] + b[index_x+1][index_y+1]) / 6.0;
    const float E = (-b[index_x-1][index_y-1] - b[index_x][index_y-1] - b[index_x+1][index_y-1] + b[index_x-1][index_y+1] + b[index_x][index_y+1] + b[index_x+1][index_y+1]) / 6.0;
    const float F = (-b[index_x-1][index_y-1] + 2 * b[index_x][index_y-1] - b[index_x+1][index_y-1] + 2 * b[index_x-1][index_y] + 5 * b[index_x][index_y] + 2 * b[index_x+1][index_y] - b[index_x-1][index_y+1] +
                    2 * b[index_x][index_y+1] - b[index_x+1][index_y+1]) / 9.0;

    return (float2)(movement.x + (B * E - 2.0 * C * D) / (4.0 * A * C - B * B),
                    movement.y + (B * D - 2.0 * A * E) / (4.0 * A * C - B * B));
}

inline float2 findMovementNCC(
        __read_only image2d_t previousFrame,
        __read_only image2d_t currentFrame,
        const int2 pos,
        float targetMean
        ) {
    // Create grid for subpixel movement calculations
    float b[GRID_SIZE][GRID_SIZE];

    // For every possible block position
    float bestScore = -1.0f;
    float2 movement = {0, 0};
    for(int y = pos.y - SEARCH_SIZE - 1; y <= pos.y + SEARCH_SIZE + 1; ++y)  {
        for(int x = pos.x - SEARCH_SIZE - 1; x <= pos.x + SEARCH_SIZE + 1; ++x)  {
            // previousframe at pos x,y is the current candidate
            const float candidateMean = calculateMeanIntensity(previousFrame, (int2)(x, y));
            float upperPart = 0.0f;
            float lowerPart1 = 0.0f;
            float lowerPart2 = 0.0f;
            // Loop over target and candidate block
            for(int a = -BLOCK_SIZE; a <= BLOCK_SIZE; ++a) {
                for(int b = -BLOCK_SIZE; b <= BLOCK_SIZE; ++b) {
                    const float imagePart = getPixelAsFloat(previousFrame, (int2)(x+a, y+b)) - candidateMean;
                    const float targetPart = getPixelAsFloat(currentFrame, (int2)(pos.x+a, pos.y+b)) - targetMean;
                    upperPart += imagePart*targetPart;
                    lowerPart1 += imagePart*imagePart;
                    lowerPart2 += targetPart*targetPart;
                }
            }

            const float result = upperPart / sqrt(lowerPart1*lowerPart2);
            b[x- pos.x + SEARCH_SIZE + 1][y - pos.y + SEARCH_SIZE + 1] = result;
            if(result > bestScore && abs(x - pos.x) <= SEARCH_SIZE && abs(y - pos.y) <= SEARCH_SIZE) {
                bestScore = result;
                movement = (float2)(x - pos.x, y - pos.y); // Movement is the offset from pos.x, pos.y
            }
        }
    }

    float2 subpixel_movement = findSubpixelMovement(movement, b);
    return subpixel_movement;
}

__kernel void normalizedCrossCorrelation(
        __read_only image2d_t previousFrame,
        __read_only image2d_t currentFrame,
        __write_only image2d_t output,
        __private const float intensityThreshold,
        __private const float timeLag,
        __private const char forwardBackward
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    // Template is what we are looking for (target), currentFrame at pos
    const float targetMean = calculateMeanIntensity(currentFrame, pos);
    if(targetMean < intensityThreshold) { // if target is all zero/black, just stop here
        write_imagef(output, pos, (float4)(0, 0, 0, 0));
        return;
    }

    float2 movement = findMovementNCC(previousFrame, currentFrame, pos, targetMean);
    if(forwardBackward == 1) {
      const float targetMean2 = calculateMeanIntensity(previousFrame, pos);
      movement = (movement - findMovementNCC(currentFrame, previousFrame, pos, targetMean2))*0.5f;
    }

    // If movement is larger than SEARCH_SIZE, zero it out
    if(length(movement) > SEARCH_SIZE + 1)
        movement = (float2)(0,0);

    write_imagef(output, pos, movement.xyyy / timeLag);
}

inline float2 findMovementSSD(
        __read_only image2d_t previousFrame,
        __read_only image2d_t currentFrame,
        const int2 pos,
        float minIntensity,
        float maxIntensity
        ) {
    // Create grid for subpixel movement calculations
    float b[GRID_SIZE][GRID_SIZE];

    // For every possible block position
    float bestScore = 0.0f;
    float2 movement = {0, 0};
    for(int y = pos.y - SEARCH_SIZE - 1; y <= pos.y + SEARCH_SIZE + 1; ++y)  {
        for(int x = pos.x - SEARCH_SIZE - 1; x <= pos.x + SEARCH_SIZE + 1; ++x)  {
            // previousframe at pos x,y is the current candidate
            float ssd = 0.0f;
            // Loop over target and candidate block
            for(int a = -BLOCK_SIZE; a <= BLOCK_SIZE; ++a) {
                for(int b = -BLOCK_SIZE; b <= BLOCK_SIZE; ++b) {
                    float val = (getPixelAsFloat(previousFrame, (int2)(x+a,y+b)) - minIntensity)/(maxIntensity - minIntensity) -
                            (getPixelAsFloat(currentFrame, (int2)(pos.x+a,pos.y+b)) - minIntensity)/(maxIntensity - minIntensity);
                    ssd += val*val;
                }
            }

            const float result = 1.0f - (ssd/((BLOCK_SIZE*2+1)*(BLOCK_SIZE*2+1))); // calculate average and invert

            b[x- pos.x + SEARCH_SIZE + 1][y - pos.y + SEARCH_SIZE + 1] = result;
            if(result > bestScore && abs(x - pos.x) <= SEARCH_SIZE && abs(y - pos.y) <= SEARCH_SIZE) {
                bestScore = result;
                movement = (float2)(x - pos.x, y - pos.y); // Movement is the offset from pos.x, pos.y
            }
        }
    }
    float2 subpixel_movement = findSubpixelMovement(movement, b);
    return subpixel_movement;
}

__kernel void sumOfSquaredDifferences(
        __read_only image2d_t previousFrame,
        __read_only image2d_t currentFrame,
        __write_only image2d_t output,
        __private const float intensityThreshold,
        __private const float timeLag,
        __private const char forwardBackward,
        __private const float minIntensity,
        __private const float maxIntensity
) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    // Template is what we are looking for (target), currentFrame at pos
    const float targetMean = calculateMeanIntensity(currentFrame, pos);
    if(targetMean < intensityThreshold) { // if target is all zero/black, just stop here
        write_imagef(output, pos, (float4)(0, 0, 0, 0));
        return;
    }

    float2 movement = findMovementSSD(previousFrame, currentFrame, pos, minIntensity, maxIntensity);
    if(forwardBackward == 1)
      movement = (movement - findMovementSSD(currentFrame, previousFrame, pos, minIntensity, maxIntensity))*0.5f;

    // If movement is larger than SEARCH_SIZE, zero it out
    if(length(movement) > SEARCH_SIZE + 1)
        movement = (float2)(0,0);

    write_imagef(output, pos, movement.xyyy / timeLag);
}

inline float2 findMovementSAD(
        __read_only image2d_t previousFrame,
        __read_only image2d_t currentFrame,
        const int2 pos,
        float minIntensity,
        float maxIntensity
        ) {
    // Create grid for subpixel movement calculations
    float b[GRID_SIZE][GRID_SIZE];

    // For every possible block position
    float bestScore = 0.0f;
    float2 movement = {0, 0};
    for(int y = pos.y - SEARCH_SIZE - 1; y <= pos.y + SEARCH_SIZE + 1; ++y)  {
        for(int x = pos.x - SEARCH_SIZE - 1; x <= pos.x + SEARCH_SIZE + 1; ++x)  {
            // previousframe at pos x,y is the current candidate
            float sad = 0.0f;
            // Loop over target and candidate block
            for(int a = -BLOCK_SIZE; a <= BLOCK_SIZE; ++a) {
                for(int b = -BLOCK_SIZE; b <= BLOCK_SIZE; ++b) {
                    sad += fabs((getPixelAsFloat(previousFrame, (int2)(x+a,y+b)) - minIntensity)/(maxIntensity - minIntensity) -
                            (getPixelAsFloat(currentFrame, (int2)(pos.x+a,pos.y+b)) - minIntensity)/(maxIntensity - minIntensity));
                }
            }

            const float result = 1.0f - (sad/((BLOCK_SIZE*2+1)*(BLOCK_SIZE*2+1))); // calculate average and invert

            b[x- pos.x + SEARCH_SIZE + 1][y - pos.y + SEARCH_SIZE + 1] = result;
            if(result > bestScore && abs(x - pos.x) <= SEARCH_SIZE && abs(y - pos.y) <= SEARCH_SIZE) {
                bestScore = result;
                movement = (float2)(x - pos.x, y - pos.y); // Movement is the offset from pos.x, pos.y
            }
        }
    }

    float2 subpixel_movement = findSubpixelMovement(movement, b);
    return subpixel_movement;
}

__kernel void sumOfAbsoluteDifferences(
        __read_only image2d_t previousFrame,
        __read_only image2d_t currentFrame,
        __write_only image2d_t output,
        __private const float intensityThreshold,
        __private const float timeLag,
        __private const char forwardBackward,
        __private const float minIntensity,
        __private const float maxIntensity
) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    // Template is what we are looking for (target), currentFrame at pos
    const float targetMean = calculateMeanIntensity(currentFrame, pos);
    if(targetMean < intensityThreshold) { // if target is all zero/black, just stop here
        write_imagef(output, pos, (float4)(0, 0, 0, 0));
        return;
    }

    float2 movement = findMovementSAD(previousFrame, currentFrame, pos, minIntensity, maxIntensity);
    if(forwardBackward == 1)
      movement = (movement - findMovementSAD(currentFrame, previousFrame, pos, minIntensity, maxIntensity))*0.5f;

    // If movement is larger than SEARCH_SIZE, zero it out
    if(length(movement) > SEARCH_SIZE + 1)
        movement = (float2)(0,0);

    write_imagef(output, pos, movement.xyyy / timeLag);
}
