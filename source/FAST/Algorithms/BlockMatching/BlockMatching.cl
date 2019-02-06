__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

float getPixelAsFloat(__read_only image2d_t image, int2 pos) {
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

float calculateMeanIntensity(__read_only image2d_t frame, int2 pos, int blockSize) {
    float mean = 0.0f;
    for(int y = pos.y - blockSize; y <= pos.y + blockSize; ++y) {
        for(int x = pos.x - blockSize; x <= pos.x + blockSize; ++x) {
            mean += getPixelAsFloat(frame, (int2)(x,y));
        }
    }

    return mean/((blockSize*2+1)*(blockSize*2+1));
}


inline float2 findSubpixelMovement(const int2 movement, const int searchSize, const float b[SEARCH_SIZE][SEARCH_SIZE]) {

    const int index_x = movement.x + searchSize+1;
    const int index_y = movement.y + searchSize+1;
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

    return (float2)((float)movement.x + (B * E - 2.0 * C * D) / (4.0 * A * C - B * B),
                                  (float)movement.y + (B * D - 2.0 * A * E) / (4.0 * A * C - B * B));
}

__kernel void normalizedCrossCorrelation(
        __read_only image2d_t previousFrame,
        __read_only image2d_t currentFrame,
        __write_only image2d_t output,
        __private const int blockSize, // block size in half
        __private const int searchSize,
        __private const float intensityThreshold
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    // Template is what we are looking for (target), currentFrame at pos
    const float targetMean = calculateMeanIntensity(currentFrame, pos, blockSize);
    if(targetMean < intensityThreshold) { // if target is all zero/black, just stop here
        write_imagef(output, pos, (float4)(0, 0, 0, 0));
        return;
    }

    // Create grid for subpixel movement calculations
    float b[SEARCH_SIZE][SEARCH_SIZE];

    // For every possible block position
    float bestScore = -1.0f;
    int2 movement = {0, 0};
    for(int y = pos.y - searchSize - 1; y <= pos.y + searchSize + 1; ++y)  {
        for(int x = pos.x - searchSize - 1; x <= pos.x + searchSize + 1; ++x)  {
            // previousframe at pos x,y is the current candidate
            const float candidateMean = calculateMeanIntensity(previousFrame, (int2)(x, y), blockSize);
            float upperPart = 0.0f;
            float lowerPart1 = 0.0f;
            float lowerPart2 = 0.0f;
            // Loop over target and candidate block
            for(int a = -blockSize; a <= blockSize; ++a) {
                for(int b = -blockSize; b <= blockSize; ++b) {
                    const float imagePart = getPixelAsFloat(previousFrame, (int2)(x+a, y+b)) - candidateMean;
                    const float targetPart = getPixelAsFloat(currentFrame, (int2)(pos.x+a, pos.y+b)) - targetMean;
                    upperPart += imagePart*targetPart;
                    lowerPart1 += imagePart*imagePart;
                    lowerPart2 += targetPart*targetPart;
                }
            }

            const float result = upperPart / sqrt(lowerPart1*lowerPart2);
            b[x- pos.x + searchSize + 1][y - pos.y + searchSize + 1] = result;
            if(result > bestScore && abs(x - pos.x) <= searchSize && abs(y - pos.y) <= searchSize) {
                bestScore = result;
                movement = (int2)(x - pos.x, y - pos.y); // Movement is the offset from pos.x, pos.y
            }
        }
    }

    float2 subpixel_movement = findSubpixelMovement(movement, searchSize, b);

    // If movement is larger than searchSize, zero it out
    if(length(subpixel_movement) > searchSize + 1) {
        write_imagef(output, pos, (float4)(0, 0, 0, 0));
        return;
    }

    write_imagef(output, pos, subpixel_movement.xyyy);
}

__kernel void sumOfSquaredDifferences(
        __read_only image2d_t previousFrame,
        __read_only image2d_t currentFrame,
        __write_only image2d_t output,
        __private const int blockSize, // block size in half
        __private const int searchSize,
        __private const float intensityThreshold,
        __private const float minIntensity,
        __private const float maxIntensity
) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    // Template is what we are looking for (target), currentFrame at pos
    const float targetMean = calculateMeanIntensity(currentFrame, pos, blockSize);
    if(targetMean < intensityThreshold) { // if target is all zero/black, just stop here
        write_imagef(output, pos, (float4)(0, 0, 0, 0));
        return;
    }

    // Create grid for subpixel movement calculations
    float b[SEARCH_SIZE][SEARCH_SIZE];

    // For every possible block position
    float bestScore = 0.0f;
    int2 movement = {0, 0};
    for(int y = pos.y - searchSize - 1; y <= pos.y + searchSize + 1; ++y)  {
        for(int x = pos.x - searchSize - 1; x <= pos.x + searchSize + 1; ++x)  {
            // previousframe at pos x,y is the current candidate
            float ssd = 0.0f;
            // Loop over target and candidate block
            for(int a = -blockSize; a <= blockSize; ++a) {
                for(int b = -blockSize; b <= blockSize; ++b) {
                    float val = (getPixelAsFloat(previousFrame, (int2)(x+a,y+b)) - minIntensity)/(maxIntensity - minIntensity) -
                            (getPixelAsFloat(currentFrame, (int2)(pos.x+a,pos.y+b)) - minIntensity)/(maxIntensity - minIntensity);
                    ssd += val*val;
                }
            }

            const float result = 1.0f - (ssd/((blockSize*2+1)*(blockSize*2+1))); // calculate average and invert

            b[x- pos.x + searchSize + 1][y - pos.y + searchSize + 1] = result;
            if(result > bestScore && abs(x - pos.x) <= searchSize && abs(y - pos.y) <= searchSize) {
                bestScore = result;
                movement = (int2)(x - pos.x, y - pos.y); // Movement is the offset from pos.x, pos.y
            }
        }
    }

    float2 subpixel_movement = findSubpixelMovement(movement, searchSize, b);

    // If movement is larger than searchSize, zero it out
    if(length(subpixel_movement) > searchSize + 1) {
        write_imagef(output, pos, (float4)(0, 0, 0, 0));
        return;
    }

    write_imagef(output, pos, subpixel_movement.xyyy);
}


__kernel void sumOfAbsoluteDifferences(
        __read_only image2d_t previousFrame,
        __read_only image2d_t currentFrame,
        __write_only image2d_t output,
        __private const int blockSize, // block size in half
        __private const int searchSize,
        __private const float intensityThreshold,
        __private const float minIntensity,
        __private const float maxIntensity
) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    // Template is what we are looking for (target), currentFrame at pos
    const float targetMean = calculateMeanIntensity(currentFrame, pos, blockSize);
    if(targetMean < intensityThreshold) { // if target is all zero/black, just stop here
        write_imagef(output, pos, (float4)(0, 0, 0, 0));
        return;
    }

    // Create grid for subpixel movement calculations
    float b[SEARCH_SIZE][SEARCH_SIZE];

    // For every possible block position
    float bestScore = 0.0f;
    int2 movement = {0, 0};
    for(int y = pos.y - searchSize - 1; y <= pos.y + searchSize + 1; ++y)  {
        for(int x = pos.x - searchSize - 1; x <= pos.x + searchSize + 1; ++x)  {
            // previousframe at pos x,y is the current candidate
            float sad = 0.0f;
            // Loop over target and candidate block
            for(int a = -blockSize; a <= blockSize; ++a) {
                for(int b = -blockSize; b <= blockSize; ++b) {
                    sad += fabs((getPixelAsFloat(previousFrame, (int2)(x+a,y+b)) - minIntensity)/(maxIntensity - minIntensity) -
                            (getPixelAsFloat(currentFrame, (int2)(pos.x+a,pos.y+b)) - minIntensity)/(maxIntensity - minIntensity));
                }
            }

            const float result = 1.0f - (sad/((blockSize*2+1)*(blockSize*2+1))); // calculate average and invert

            b[x- pos.x + searchSize + 1][y - pos.y + searchSize + 1] = result;
            if(result > bestScore && abs(x - pos.x) <= searchSize && abs(y - pos.y) <= searchSize) {
                bestScore = result;
                movement = (int2)(x - pos.x, y - pos.y); // Movement is the offset from pos.x, pos.y
            }
        }
    }

    float2 subpixel_movement = findSubpixelMovement(movement, searchSize, b);

    // If movement is larger than searchSize, zero it out
    if(length(subpixel_movement) > searchSize + 1) {
        write_imagef(output, pos, (float4)(0, 0, 0, 0));
        return;
    }

    write_imagef(output, pos, subpixel_movement.xyyy);
}
