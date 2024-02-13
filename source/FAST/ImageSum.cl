
#ifdef TYPE_FLOAT
#define TYPE float4
#define BUFFER_TYPE float
#define READ_IMAGE read_imagef
#define WRITE_IMAGE write_imagef
#elif TYPE_UINT8
#define TYPE uint4
#define BUFFER_TYPE uchar
#define READ_IMAGE read_imageui
#define WRITE_IMAGE write_imageui
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
#elif TYPE_INT16
#define TYPE int4
#define BUFFER_TYPE short
#define READ_IMAGE read_imagei
#define WRITE_IMAGE write_imagei
#elif TYPE_UINT32
#define TYPE uint4
#define BUFFER_TYPE uint
#define READ_IMAGE read_imageui
#define WRITE_IMAGE write_imageui
#else
#define TYPE int4
#define BUFFER_TYPE int
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
        int2 nPos = readPos + offset2D[i];
        if(nPos.x < size.x && nPos.y < size.y) {
            sum += READ_IMAGE(image, sampler, nPos).x;
        }
    }

    write_imagef(firstLevel, pos, sum);
}

__kernel void createFirstSumImage3DLevel(
        __read_only image3d_t image,
        __global float* firstLevel
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int4 readPos = pos*2;
    const int3 size = {get_image_width(image), get_image_height(image), get_image_depth(image)};

    float sum = 0.0f;
    for(int i = 0; i < 8; ++i) {
        int4 nPos = readPos + offset3D[i];
        if(nPos.x < size.x && nPos.y < size.y && nPos.z < size.z) {
            sum += READ_IMAGE(image, sampler, nPos).x;
        }
    }

    firstLevel[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = sum;
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

__kernel void createSumImage3DLevel(
        __global float* readLevel,
        __global float* writeLevel
        ) {
    const int3 pos = {get_global_id(0), get_global_id(1), get_global_id(2)};
    const int3 readPos = pos*2;
    const int3 size = {get_global_size(0), get_global_size(1), get_global_size(2)};
    const int3 readSize = size*2;

    float sum = 0.0f;
    for(int i = 0; i < 8; i++) {
        int3 nPos = readPos + offset3D[i].xyz;
        sum += readLevel[nPos.x+nPos.y*readSize.x+nPos.z*readSize.x*readSize.y];
    }

    writeLevel[pos.x+pos.y*size.x+pos.z*size.x*size.y] = sum;
}


__kernel void createFirstStdDevImage2DLevel(
        __read_only image2d_t image,
        __write_only image2d_t firstLevel,
        __private float average
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    const int2 readPos = pos*2;
    const int2 size = {get_image_width(image), get_image_height(image)};

    float sum = 0.0f;
    for(int i = 0; i < 4; i++) {
        int2 nPos = readPos + offset2D[i];
        if(nPos.x < size.x && nPos.y < size.y) {
            sum += pow(READ_IMAGE(image, sampler, nPos).x - average, 2.0f);
        }
    }

    write_imagef(firstLevel, pos, sum);
}

__kernel void createFirstStdDevImage3DLevel(
        __read_only image3d_t image,
        __global float* firstLevel,
        __private float average
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int4 readPos = pos*2;
    const int3 size = {get_image_width(image), get_image_height(image), get_image_depth(image)};

    float sum = 0.0f;
    for(int i = 0; i < 8; i++) {
        int4 nPos = readPos + offset3D[i];
        if(nPos.x < size.x && nPos.y < size.y && nPos.z < size.z) {
            sum += pow(READ_IMAGE(image, sampler, nPos).x - average, 2.0f);
        }
    }

    firstLevel[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = sum;
}
