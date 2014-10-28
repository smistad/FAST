
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

void checkNeighborhood(
        const int2 position,
        __read_only image2d_t readImage,
        __write_only image2d_t writeImage,
        __global char* stop,
        uchar add
        ) {
    const int2 offsets[9] = {
            {0,0},
            {0,1},
            {1,1},
            {1,0},
            {1,-1},
            {0,-1},
            {-1,-1},
            {-1,0},
            {-1,1}
    };
    // Read all neighbors
    uchar neighbors[9];
    neighbors[0] = read_imageui(readImage, sampler, position).x;
    if(neighbors[0] == 1) { // check if current pixel is background
        uchar sum = 0;
        uchar transitions = 0; // number of 0-1 transitions
        bool previousWasZero = false;
        for(uchar i = 1; i < 9; i++) {
            neighbors[i] = read_imageui(readImage, sampler, position+offsets[i]).x;
            sum += neighbors[i];
            if(previousWasZero && neighbors[i] == 1) {
                transitions++;
            }
            if(neighbors[i] == 0) {
                previousWasZero = true;
            } else {
                previousWasZero = false;
            }
        }
        // Check last one
        if(previousWasZero && neighbors[1] == 1) {
            transitions++;
        }
        
        if(sum >= 2 && sum <= 6 &&
                transitions == 1 &&
                neighbors[1]*neighbors[3]*neighbors[5+add] == 0 &&
                neighbors[3-add]*neighbors[5]*neighbors[7] == 0) {
            // Delete pixel
            write_imageui(writeImage, position, 0);
            stop[0] = 0; // A pixel was deleted, continue
        } else {
            write_imageui(writeImage, position, 1);
        }
    } else {
        write_imageui(writeImage, position, 0);
    }
}

__kernel void thinningStep1(
        __read_only image2d_t readImage,
        __write_only image2d_t writeImage,
        __global char* stop
        ) {
    const int2 position = {get_global_id(0), get_global_id(1)};
    checkNeighborhood(position, readImage, writeImage, stop, 0);
}

__kernel void thinningStep2(
        __read_only image2d_t readImage,
        __write_only image2d_t writeImage,
        __global char* stop
        ) {
    const int2 position = {get_global_id(0), get_global_id(1)};
    checkNeighborhood(position, readImage, writeImage, stop, 2);
}