__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
__constant sampler_t sampler2 = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)

__kernel void convertToHU(
	__read_only image3d_t input,
#ifdef fast_3d_image_writes
	__write_only image3d_t output
#else
	__global short* output
#endif
	) {
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	
	int value = read_imageui(input, sampler, pos).x;
	value -= 1024;
#ifdef fast_3d_image_writes
	write_imagei(output, pos, value);
#else
	output[LPOS(pos)] = value;
#endif
}

__kernel void dilate(
        __read_only image3d_t volume, 
#ifdef fast_3d_image_writes
        __write_only image3d_t result
#else
        __global uchar * result
#endif
    ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    if(read_imageui(volume, sampler, pos).x == 1) {
    	const int N = 2;
        for(int a = -N; a <= N; ++a) {
        for(int b = -N; b <= N; ++b) {
        for(int c = -N; c <= N; ++c) {
            int4 nPos = pos + (int4)(a,b,c,0);
#ifdef fast_3d_image_writes
            write_imageui(result, nPos, 1);
#else
            // Check if in bounds
            if(nPos.x >= 0 && nPos.y >= 0 && nPos.z >= 0 &&
                nPos.x < get_global_size(0) && nPos.y < get_global_size(1) && nPos.z < get_global_size(2))
            result[LPOS(nPos)] = 1;
#endif
        }}}
    }
    
}

__kernel void erode(
        __read_only image3d_t volume, 
#ifdef fast_3d_image_writes
        __write_only image3d_t result
#else
        __global uchar * result
#endif
    ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    int value = read_imageui(volume, sampler, pos).x;
    if(value == 1) {
        bool keep = true;
    	const int N = 2;
    	// Check if all pixels in neighborhood is 1
        for(int a = -N; a <= N; ++a) {
        for(int b = -N; b <= N; ++b) {
        for(int c = -N; c <= N; ++c) {
            keep = (read_imageui(volume, sampler2, pos + (int4)(a,b,c,0)).x == 1 && keep);
        }}}
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