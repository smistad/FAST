__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)

__kernel void initialize(
	__read_only image3d_t input,
#ifdef fast_3d_image_writes
	__write_only image3d_t distance
#else
	__global short* distance
#endif
	) {
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	
	uint value = read_imageui(input, sampler, pos).x;
	if(value == 0) {
		// Outside
#ifdef fast_3d_image_writes
		write_imagei(distance, pos, 0);
#else
		distance[LPOS(pos)] = 0;
#endif
	} else {
		// Inside
		bool atBorder = false;
        for(int a = -1; a <= 1;  ++a) {
        for(int b = -1; b <= 1;  ++b) {
        for(int c = -1; c <= 1;  ++c) {
        	int4 nPos = (int4)(a,b,c,0) + pos;
            uint value2 = read_imageui(input, sampler, nPos).x;
            if(value2 == 0) {
                atBorder = true;
            }
        }}}
        if(atBorder) {
#ifdef fast_3d_image_writes
            write_imagei(distance, pos, -1);
#else
			distance[LPOS(pos)] = -1;
#endif
        } else {
#ifdef fast_3d_image_writes
            write_imagei(distance, pos, -1);
#else
			distance[LPOS(pos)] = -1;
#endif
        }
	}
}

__constant int4 neighbors[] = {
	{1, 0, 0, 0},
	{-1, 0, 0, 0},
	{0, 1, 0, 0},
	{0, -1, 0, 0},
	{0, 0, 1, 0},
	{0, 0, -1, 0}
};

__kernel void calculateDistance(
#ifdef fast_3d_image_writes
	__read_only image3d_t input,
	__write_only image3d_t output,
#else
	__global short* input,
	__global short* output,
#endif
	__global char* changed
	) {
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
#ifdef fast_3d_image_writes
	int value = read_imagei(input, sampler, pos).x;
#else
	int value = input[LPOS(pos)];
#endif
	if(value == -1) { // -1 means no distance calculated yet
		int minNeighborDistance = 999999;
        for(int i = 0; i < 6; ++i) {
        	int4 nPos = neighbors[i] + pos;
#ifdef fast_3d_image_writes
			int value2 = read_imagei(input, sampler, nPos).x;
#else
			int value2 = input[LPOS(nPos)];
#endif
        	if(value2 < minNeighborDistance && value2 >= 0) {
        		minNeighborDistance = value2;
        	}
        }
        
        if(minNeighborDistance < 999999) {
        	// Valid neighbor found
#ifdef fast_3d_image_writes
        	write_imagei(output, pos, minNeighborDistance+1);
#else
			output[LPOS(pos)] = minNeighborDistance+1;
#endif
        	changed[0] = 1;
        }
	} else {
#ifdef fast_3d_image_writes
        write_imagei(output, pos, value);
#else
		output[LPOS(pos)] = value;
#endif
    }
}

__kernel void findCandidateCenterpoints(
			__read_only image3d_t segmentation,
			__read_only image3d_t distanceImage,
#ifdef fast_3d_image_writes
			__write_only image3d_t output
#else
			__global uchar* output
#endif
        ) {
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    if(read_imageui(segmentation, sampler, pos).x == 1) {
        // Inside object
        short distance = read_imagei(distanceImage, sampler, pos).x;

        // Check if voxel is candidate centerline
        int N = 4;
        bool invalid = false;
        for(int a = -N; a <= N;  ++a) {
        for(int b = -N; b <= N;  ++b) {
        for(int c = -N; c <= N;  ++c) {
            short distance2 = read_imagei(distanceImage, sampler, pos + (int4)(a,b,c,0)).x;
            if(distance2 > distance) {
                invalid = true;
            }
        }}}

#ifdef fast_3d_image_writes
        if(!invalid) {
        	write_imageui(output, pos, 1);
        } else {
        	write_imageui(output, pos, 0);
        }
    } else {
        write_imageui(output, pos, 0);
    }
#else
        if(!invalid) {
			output[LPOS(pos)] = 1;
        } else {
			output[LPOS(pos)] = 0;
        }
    } else {
		output[LPOS(pos)] = 0;
    }
#endif
}