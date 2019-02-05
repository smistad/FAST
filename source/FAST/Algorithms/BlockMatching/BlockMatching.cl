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

float3 Hue(float H)
{
    float R = fabs(H * 6.0f - 3.0f) - 1;
    float G = 2 - fabs(H * 6 - 2);
    float B = 2 - fabs(H * 6 - 4);
    return clamp((float3)(R, G, B), 0.0f, 1.0f);
}

#define searchSizeConst 7 // Should be searchSize*2 + 1 + 2

__kernel void blockMatching(
        __read_only image2d_t previousFrame,
        __read_only image2d_t currentFrame,
        __write_only image2d_t output,
        __private const int blockSize, // block size in half
        __private const int searchSize
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    const int width = get_image_width(currentFrame);
    const int height = get_image_height(currentFrame);

    // Template is what we are looking for (target), currentFrame at pos
    const float targetMean = calculateMeanIntensity(currentFrame, pos, blockSize);
    if(targetMean < 75.0f) { // if target is all zero/black, just stop here
        write_imagef(output, pos, (float4)(0, 0, 0, 0));
        return;
    }

    // Create 3x3 grid for subpixel fitting
    float b[searchSizeConst][searchSizeConst];
    for(int x = 0; x < searchSizeConst; x++) {
        for(int y = 0; y < searchSizeConst; y++) {
            b[x][y] = 0;
        }
    }

    // For every possible block position
    float bestScore = -1;
    int2 movement = {0, 0};
    for(int y = pos.y - searchSize; y <= pos.y + searchSize; ++y)  {
        for(int x = pos.x - searchSize; x <= pos.x + searchSize; ++x)  {
            // previousframe at pos x,y is the current candidate
            const float candidateMean = calculateMeanIntensity(previousFrame, (int2)(x, y), blockSize);
            float upperPart = 0.0f;
            float lowerPart1 = 0.0f;
            float lowerPart2 = 0.0f;
            float ssd = 0.0f;
            // Loop over target and candidate block
            for(int a = -blockSize; a <= blockSize; ++a) {
                for(int b = -blockSize; b <= blockSize; ++b) {
                    const float imagePart = getPixelAsFloat(previousFrame, (int2)(x+a, y+b)) - candidateMean;
                    const float targetPart = getPixelAsFloat(currentFrame, (int2)(pos.x+a, pos.y+b)) - targetMean;
                    upperPart += imagePart*targetPart;
                    lowerPart1 += imagePart*imagePart;
                    lowerPart2 += targetPart*targetPart;
                    // TODO assuming uint8 images, with max value 255. should have this has input
                    float val = getPixelAsFloat(previousFrame, (int2)(x+a,y+b))/255.0f - getPixelAsFloat(currentFrame, (int2)(pos.x+a,pos.y+b))/255.0f; // SSD
                    //float val = fabs(getPixelAsFloat(previousFrame, (int2)(x+a,y+b))/255.0f - getPixelAsFloat(currentFrame, (int2)(pos.x+a,pos.y+b))/255.0f); // SAD
                    ssd += val*val;
                }
            }

            const float result = upperPart / sqrt(lowerPart1*lowerPart2);
            //const float result = 1.0f - (ssd/((blockSize*2+1)*(blockSize*2+1)));

            b[x-pos.x+searchSize + 1][y-pos.y+searchSize + 1] = result;
            if(result > bestScore) {
                bestScore = result;
                movement = (int2)(x - pos.x, y - pos.y); // Movement is the offset from pos.x, pos.y
            }
        }
    }
    /*
    for(int x = 0; x < 7; x++) {
        for(int y = 0; y < 7; y++) {
            printf("%f\n", b[x][y]);
        }
    }*/

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

    float2 subpixel_movement = {(float)movement.x + (B * E - 2.0 * C * D) / (4.0 * A * C - B * B),
                                  (float)movement.y + (B * D - 2.0 * A * E) / (4.0 * A * C - B * B)};

    //subpixel_movement = (float2)(movement.x, movement.y);
    //printf("%d %d - %f %f\n", movement.x, movement.y, subpixel_movement.x, subpixel_movement.y);

    //printf("best score %f\n", bestScore);
    //printf("%f %f\n", (float)movement.x/searchSize, (float)movement.y/searchSize);
    float velDir = (1.0f + atan2((float)subpixel_movement.x, (float)subpixel_movement.y) / 3.141592f) / 2.0f;
    float magnitude = length(subpixel_movement) / sqrt((float)searchSize*searchSize + searchSize*searchSize);
    //printf("%f %f %f\n", magnitude, (float)movement.x/searchSize, (float)movement.y/searchSize);
    float3 color = Hue(velDir)*magnitude;
    write_imagef(output, pos, color.xyzz);
}