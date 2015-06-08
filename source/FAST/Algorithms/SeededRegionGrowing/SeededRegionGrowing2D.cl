
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;

#ifdef TYPE_FLOAT
#define READ_IMAGE(image, pos) read_imagef(image, sampler, pos).x
#elif TYPE_INT
#define READ_IMAGE(image, pos) (float)read_imagei(image, sampler, pos).x
#else
#define READ_IMAGE(image, pos) (float)read_imageui(image, sampler, pos).x
#endif

__kernel void seededRegionGrowing(
        __read_only image2d_t image,
        __global char* segmentation,
        __global char* stopGrowing,
        __private float min,
        __private float max
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    const uint linearPos = pos.x + pos.y*get_global_size(0);
    
    // Define neighborhood offsets
    int2 offset[8] = {
            {1,0},
            {0,1},
            {1,1},
            {-1,0},
            {0,-1},
            {-1,-1},
            {-1,1},
            {1,-1}
    };
    
    if(segmentation[linearPos] == 2) { // pixel is in queue
        float intensity = READ_IMAGE(image, pos);
        if(intensity >= min && intensity <= max) {
            segmentation[linearPos] = 1; // add pixel to segmentation
            // Add neighbor pixels to queue
            for(int i = 0; i < 8; i++) {
                // Out of bounds check
                int2 neighborPos = pos + offset[i];
                if(neighborPos.x < 0 || neighborPos.y < 0 || 
                    neighborPos.x >= get_global_size(0) || neighborPos.y >= get_global_size(1))
                    continue;
                uint neighborLinearPos = neighborPos.x + neighborPos.y*get_global_size(0);
                if(segmentation[neighborLinearPos] == 0) {
                    segmentation[neighborLinearPos] = 2; // add to queue
                    stopGrowing[0] = 0;
                }
            }
        } else {
            segmentation[linearPos] = 0; // Remove pixel
        }
    }
}
        
            