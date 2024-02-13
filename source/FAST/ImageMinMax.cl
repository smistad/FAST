
#ifdef TYPE_FLOAT
#define TYPE float4
#define BUFFER_TYPE float
#define READ_IMAGE read_imagef
#define WRITE_IMAGE write_imagef
#define MAX_VALUE FLT_MAX
#define MIN_VALUE FLT_MIN
#elif TYPE_UINT8
#define TYPE uint4
#define BUFFER_TYPE uchar
#define READ_IMAGE read_imageui
#define WRITE_IMAGE write_imageui
#define MAX_VALUE UCHAR_MAX
#define MIN_VALUE 0
#elif TYPE_INT8
#define TYPE int4
#define BUFFER_TYPE char
#define READ_IMAGE read_imagei
#define WRITE_IMAGE write_imagei
#define MAX_VALUE CHAR_MAX
#define MIN_VALUE CHAR_MIN
#elif TYPE_UINT16
#define TYPE uint4
#define BUFFER_TYPE ushort
#define READ_IMAGE read_imageui
#define WRITE_IMAGE write_imageui
#define MAX_VALUE USHRT_MAX
#define MIN_VALUE 0
#elif TYPE_INT16
#define TYPE int4
#define BUFFER_TYPE short
#define READ_IMAGE read_imagei
#define WRITE_IMAGE write_imagei
#define MAX_VALUE SHRT_MAX
#define MIN_VALUE SHRT_MIN
#elif TYPE_UINT32
#define TYPE uint4
#define BUFFER_TYPE uint
#define READ_IMAGE read_imageui
#define WRITE_IMAGE write_imageui
#define MAX_VALUE UINT_MAX
#define MIN_VALUE 0
#else
#define TYPE int4
#define BUFFER_TYPE int
#define READ_IMAGE read_imagei
#define WRITE_IMAGE write_imagei
#define MAX_VALUE INT_MAX
#define MIN_VALUE INT_MIN
#endif

__constant int2 offset2D[4] = {
                   {0,0},
                   {0,1},
                   {1,0},
                   {1,1}
};

__constant int4 offset3D[8] = {
                   {0,0,0,0},
                   {1,0,0,0},
                   {0,1,0,0},
                   {0,0,1,0},
                   {1,0,1,0},
                   {1,1,1,0},
                   {0,1,1,0},
                   {1,1,0,0}
};

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void createFirstMinMaxImage2DLevel(
        __read_only image2d_t image,
        __write_only image2d_t firstLevel
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    const int2 readPos = pos*2;
    const int2 size = {get_image_width(image), get_image_height(image)};

    TYPE result = {MAX_VALUE, MIN_VALUE,0,0};
    for(int i = 0; i < 4; i++) {
        TYPE temp;
        temp = READ_IMAGE(image, sampler, select((int2)(0,0), readPos+offset2D[i], readPos+offset2D[i] < size));
        result.x = min(result.x, temp.x);
        result.y = max(result.y, temp.x);
    }

    WRITE_IMAGE(firstLevel, pos, result);
}

__kernel void createMinMaxImage2DLevel(
        __read_only image2d_t readLevel,
        __write_only image2d_t writeLevel
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    const int2 readPos = pos*2;

    TYPE result = {MAX_VALUE, MIN_VALUE,0,0};
    TYPE temp;
    temp = READ_IMAGE(readLevel, sampler, readPos + offset2D[0]);
    result.x = min(result.x, temp.x);
    result.y = max(result.y, temp.y);
    temp = READ_IMAGE(readLevel, sampler, readPos + offset2D[1]);
    result.x = min(result.x, temp.x);
    result.y = max(result.y, temp.y);
    temp = READ_IMAGE(readLevel, sampler, readPos + offset2D[2]);
    result.x = min(result.x, temp.x);
    result.y = max(result.y, temp.y);
    temp = READ_IMAGE(readLevel, sampler, readPos + offset2D[3]);
    result.x = min(result.x, temp.x);
    result.y = max(result.y, temp.y);

    WRITE_IMAGE(writeLevel, pos, result);
}

#ifdef fast_3d_image_writes
__kernel void createFirstMinMaxImage3DLevel(
        __read_only image3d_t image,
        __write_only image3d_t firstLevel
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int4 readPos = pos*2;
    const int4 size = {get_image_width(image), get_image_height(image), get_image_depth(image), 1};

    TYPE result = {MAX_VALUE, MIN_VALUE,0,0};
    for(int i = 0; i < 8; i++) {
        TYPE temp;
        temp = READ_IMAGE(image, sampler,  select((int4)(0,0,0,0), readPos+offset3D[i], readPos+offset3D[i] < size));
        result.x = min(result.x, temp.x);
        result.y = max(result.y, temp.x);
    }

    WRITE_IMAGE(firstLevel, pos, result);
}

__kernel void createMinMaxImage3DLevel(
        __read_only image3d_t readLevel,
        __write_only image3d_t writeLevel
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int4 readPos = pos*2;

    TYPE result = {MAX_VALUE, MIN_VALUE,0,0};
    for(int i = 0; i < 8; i++) {
        TYPE temp;
        temp = READ_IMAGE(readLevel, sampler, readPos + offset3D[i]);
        result.x = min(result.x, temp.x);
        result.y = max(result.y, temp.y);
    }


    WRITE_IMAGE(writeLevel, pos, result);
}
#endif

__kernel void reduce(
        __global BUFFER_TYPE* buffer,
        __local BUFFER_TYPE* minScratch,
        __local BUFFER_TYPE* maxScratch,
        __private int length,
        __private int X,
        __global BUFFER_TYPE* result) {

    int global_index = get_global_id(0)*X;
    BUFFER_TYPE minAccumulator = MAX_VALUE;
    BUFFER_TYPE maxAccumulator = MIN_VALUE;
    // Loop sequentially over chunks of input vector
    for(int i = 0; i < X && global_index < length; i++) {
        float element = buffer[global_index];
        minAccumulator = (minAccumulator < element) ? minAccumulator : element;
        maxAccumulator = (maxAccumulator > element) ? maxAccumulator : element;
        global_index += 1;
    }

    // Perform parallel reduction
    int local_index = get_local_id(0);
    minScratch[local_index] = minAccumulator;
    maxScratch[local_index] = maxAccumulator;
    barrier(CLK_LOCAL_MEM_FENCE);
    for(int offset = get_local_size(0) / 2; offset > 0; offset = offset / 2) {
        if(local_index < offset) {
          BUFFER_TYPE other = minScratch[local_index + offset];
          BUFFER_TYPE mine = minScratch[local_index];
          minScratch[local_index] = (mine < other) ? mine : other;
          other = maxScratch[local_index + offset];
          mine = maxScratch[local_index];
          maxScratch[local_index] = (mine > other) ? mine : other;
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if(local_index == 0) {
        result[get_group_id(0)*2] = minScratch[0];
        result[get_group_id(0)*2+1] = maxScratch[0];
    }
}
