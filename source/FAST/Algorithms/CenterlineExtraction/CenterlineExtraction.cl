#pragma OPENCL EXTENSION cl_amd_printf : enable
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void initialize(
	__read_only image3d_t input,
	__write_only image3d_t distance
	) {
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	
	uint value = read_imageui(input, sampler, pos).x;
	if(value == 0) {
		// Outside
		write_imagei(distance, pos, 0);
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
            write_imagei(distance, pos, 0);
        } else {
            write_imagei(distance, pos, -1);
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
	__read_only image3d_t input,
	__write_only image3d_t output,
	__global char* changed
	) {
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	int value = read_imagei(input, sampler, pos).x;
	if(value == -1) { // -1 means no distance calculated yet
		int minNeighborDistance = 999999;
        for(int i = 0; i < 6; ++i) {
        	int4 nPos = neighbors[i] + pos;
            int value2 = read_imagei(input, sampler, nPos).x;
        	if(value2 < minNeighborDistance && value2 >= 0) {
        		minNeighborDistance = value2;
        	}
        }
        
        if(minNeighborDistance < 999999) {
        	// Valid neighbor found
        	write_imagei(output, pos, minNeighborDistance+1);
        	changed[0] = 1;
        }
	} else {
        write_imagei(output, pos, value);
    }
}