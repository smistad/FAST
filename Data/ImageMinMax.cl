
#ifdef TYPE_FLOAT
#define TYPE float4
#define READ_IMAGE read_imagef
#define WRITE_IMAGE write_imagef
#define MAX_VALUE FLT_MAX
#define MIN_VALUE FLT_MIN
#elif TYPE_UINT8
#define TYPE uint4
#define READ_IMAGE read_imageui
#define WRITE_IMAGE write_imageui
#define MAX_VALUE UCHAR_MAX
#define MIN_VALUE 0
#elif TYPE_INT8
#define TYPE int4
#define READ_IMAGE read_imagei
#define WRITE_IMAGE write_imagei
#define MAX_VALUE CHAR_MAX
#define MIN_VALUE CHAR_MIN
#elif TYPE_UINT16
#define TYPE uint4
#define READ_IMAGE read_imageui
#define WRITE_IMAGE write_imageui
#define MAX_VALUE USHRT_MAX
#define MIN_VALUE 0
#else
#define TYPE int4
#define READ_IMAGE read_imagei
#define WRITE_IMAGE write_imagei
#define MAX_VALUE SHRT_MAX
#define MIN_VALUE SHRT_MIN
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
    // Check if in bounds
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
