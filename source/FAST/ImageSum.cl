
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
#elif TYPE_UINT16
#define TYPE uint4
#define BUFFER_TYPE ushort
#define READ_IMAGE read_imageui
#define WRITE_IMAGE write_imageui
#else
#define TYPE int4
#define BUFFER_TYPE short
#define READ_IMAGE read_imagei
#define WRITE_IMAGE write_imagei
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

__kernel void createFirstSumImage2DLevel(
        __read_only image2d_t image,
        __write_only image2d_t firstLevel
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    const int2 readPos = pos*2;
    const int2 size = {get_image_width(image), get_image_height(image)};

    float sum = 0.0f;
    for(int i = 0; i < 4; i++) {
        sum += READ_IMAGE(image, sampler, select((int2)(0,0), readPos+offset2D[i], readPos+offset2D[i] < size)).x;
    }

    write_imagef(firstLevel, pos, sum);
}

__kernel void createSumImage2DLevel(
        __read_only image2d_t readLevel,
        __write_only image2d_t writeLevel
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    const int2 readPos = pos*2;

    float sum =
    read_imagef(readLevel, sampler, readPos + offset2D[0]).x +
    read_imagef(readLevel, sampler, readPos + offset2D[1]).x +
    read_imagef(readLevel, sampler, readPos + offset2D[2]).x +
    read_imagef(readLevel, sampler, readPos + offset2D[3]).x;

    write_imagef(writeLevel, pos, sum);
}

