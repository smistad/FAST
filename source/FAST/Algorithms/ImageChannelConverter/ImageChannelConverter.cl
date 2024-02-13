__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;



__kernel void channelConvert2D(
            __read_only image2d_t input,
            __write_only image2d_t output,
            __private uchar4 removeChannelV,
            __private char reverse,
            __private int nrOfChannels
    ) {

    int offset = 0;
    int sign = 1;
    if(reverse == 1) {
        offset = (nrOfChannels-1) - (removeChannelV.x + removeChannelV.y + removeChannelV.z + removeChannelV.w);
        sign = -1;
    }
    uchar* removeChannel = (uchar*)&removeChannelV;
    const int2 pos = {get_global_id(0), get_global_id(1)};
    int type = get_image_channel_data_type(input);
    int writePos = 0;
    if(type == CLK_FLOAT) {
        float4 tmp = read_imagef(input, sampler, pos);
        float* pixel = (float*)&tmp;
        float result[4];
        for(int i = 0; i < 4; ++i) {
            if(removeChannel[i] == 0) {
                result[writePos*sign + offset] = pixel[i];
                ++writePos;
            }
        }

        write_imagef(output, pos, (float4)(result[0], result[1], result[2], result[3]));
    } else if(type == CLK_UNSIGNED_INT8 || type == CLK_UNSIGNED_INT16 || type == CLK_UNSIGNED_INT32) {
        uint4 tmp = read_imageui(input, sampler, pos);
        uint* pixel = (uint*)&tmp;
        uint result[4];
        for(int i = 0; i < 4; ++i) {
            if(removeChannel[i] == 0) {
                result[writePos*sign + offset] = pixel[i];
                ++writePos;
            }
        }

        write_imageui(output, pos, (uint4)(result[0], result[1], result[2], result[3]));
    } else {
        int4 tmp = read_imagei(input, sampler, pos);
        int* pixel = (int*)&tmp;
        int result[4];
        for(int i = 0; i < 4; ++i) {
            if(removeChannel[i] == 0) {
                result[writePos*sign + offset] = pixel[i];
                ++writePos;
            }
        }

        write_imagei(output, pos, (int4)(result[0], result[1], result[2], result[3]));
    }
}

#ifdef fast_3d_image_writes
__kernel void channelConvert3D(
            __read_only image3d_t input,
            __write_only image3d_t output,
            __private uchar4 removeChannelV,
            __private char reverse,
            __private int nrOfChannels
    ) {

    int offset = 0;
    int sign = 1;
    if(reverse == 1) {
        offset = (nrOfChannels-1) - (removeChannelV.x + removeChannelV.y + removeChannelV.z + removeChannelV.w);
        sign = -1;
    }

    uchar* removeChannel = (uchar*)&removeChannelV;
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    int type = get_image_channel_data_type(input);
    int writePos = 0;
    if(type == CLK_FLOAT) {
        float4 tmp = read_imagef(input, sampler, pos);
        float* pixel = (float*)&tmp;
        float result[4];
        for(int i = 0; i < 4; ++i) {
            if(removeChannel[i] == 0) {
                result[writePos*sign + offset] = pixel[i];
                ++writePos;
            }
        }

        write_imagef(output, pos, (float4)(result[0], result[1], result[2], result[3]));
    } else if(type == CLK_UNSIGNED_INT8 || type == CLK_UNSIGNED_INT16 || type == CLK_UNSIGNED_INT32) {
        uint4 tmp = read_imageui(input, sampler, pos);
        uint* pixel = (uint*)&tmp;
        uint result[4];
        for(int i = 0; i < 4; ++i) {
            if(removeChannel[i] == 0) {
                result[writePos*sign + offset] = pixel[i];
                ++writePos;
            }
        }

        write_imageui(output, pos, (uint4)(result[0], result[1], result[2], result[3]));
    } else {
        int4 tmp = read_imagei(input, sampler, pos);
        int* pixel = (int*)&tmp;
        int result[4];
        for(int i = 0; i < 4; ++i) {
            if(removeChannel[i] == 0) {
                result[writePos*sign + offset] = pixel[i];
                ++writePos;
            }
        }

        write_imagei(output, pos, (int4)(result[0], result[1], result[2], result[3]));
    }
}
#else
#endif
