__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)

__kernel void dilate3D(
        __read_only image3d_t volume,
#ifdef fast_3d_image_writes
        __write_only image3d_t result,
#else
        __global uchar * result,
#endif
        __private int size
    ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    if(read_imageui(volume, sampler, pos).x == 1) {
        for(int a = -size; a <= size ; a++) {
            for(int b = -size; b <= size ; b++) {
                for(int c = -size; c <= size ; c++) {
                    int4 n = (int4)(a,b,c,0);
                    if(length(convert_float4(n)) > size)
                        continue;
#ifdef fast_3d_image_writes
                    write_imageui(result, pos + n, 1);
#else
                    // Check if in bounds
                    int4 nPos = pos + n;
                    if(nPos.x >= 0 && nPos.y >= 0 && nPos.z >= 0 &&
                        nPos.x < get_global_size(0) && nPos.y < get_global_size(1) && nPos.z < get_global_size(2))
                    result[LPOS(nPos)] = 1;
#endif
                }
            }
        }
    }
}

__kernel void dilate2D(
        __read_only image2d_t volume,
        __write_only image2d_t result,
        __private int size
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    const int width = get_image_width(result);
    const int height = get_image_height(result);

    if(read_imageui(volume, sampler, pos).x == 1) {
        char write = 0;
        for(int a = -size; a <= size ; a++) {
            for(int b = -size; b <= size ; b++) {
                int2 n = (int2)(a,b);
                if(length(convert_float2(n)) <= size && pos.x+a >= 0 && pos.x+a < width && pos.y+b >= 0 && pos.y+b < height)
                    write_imageui(result, pos + n, 1);
            }
        }
    }
}
