__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)


__kernel void erode3D(
        __read_only image3d_t volume,
#ifdef fast_3d_image_writes
        __write_only image3d_t result,
#else
        __global uchar * result,
#endif
        __private int size
    ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    int value = read_imageui(volume, sampler, pos).x;
    if(value == 1) {
        bool keep = true;
        for(int a = -size; a <= size; a++) {
            for(int b = -size; b <= size; b++) {
                for(int c = -size; c <= size; c++) {
                    int4 n = (int4)(a,b,c,0);
                    if(length(convert_float4(n)) > size)
                        continue;
                    keep = (read_imageui(volume, sampler, pos + n).x == 1 && keep);
                }
            }
        }
#ifdef fast_3d_image_writes
        write_imageui(result, pos, keep ? 1 : 0);
    } else {
        write_imageui(result, pos, 0);
    }
#else
        result[LPOS(pos)] = keep ? 1 : 0;
    } else {
        result[LPOS(pos)] = 0;
    }
#endif
}

__kernel void erode2D(
        __read_only image2d_t volume,
        __write_only image2d_t result,
        __private int size
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    int value = read_imageui(volume, sampler, pos).x;
    if(value == 1) {
        bool keep = true;
        for(int a = -size; a <= size; a++) {
            for(int b = -size; b <= size; b++) {
                int2 n = (int2)(a,b);
                if(length(convert_float2(n)) > size)
                    continue;
                keep = (read_imageui(volume, sampler, pos + n).x == 1 && keep);
            }
        }
        write_imageui(result, pos, keep ? 1 : 0);
    } else {
        write_imageui(result, pos, 0);
    }
}
