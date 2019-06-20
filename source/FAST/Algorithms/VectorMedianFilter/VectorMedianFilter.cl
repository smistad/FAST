__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void vectorMedianFilter(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private int windowHalf
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float2 bestVector;
    float bestSum = FLT_MAX;
    for(int y = -windowHalf; y <= windowHalf; ++y) {
        for(int x = -windowHalf; x <= windowHalf; ++x) {
            float2 a_m = read_imagef(input, sampler, pos + (int2)(x, y)).xy;
            float sum = 0.0f;
            for(int y2 = -windowHalf; y2 <= windowHalf; ++y2) {
                for(int x2 = -windowHalf; x2 <= windowHalf; ++x2) {
                    float2 a_i = read_imagef(input, sampler, pos + (int2)(x2, y2)).xy;
                    //sum += dot(fabs(a_m - a_i), (float2)(1,1)); // L1 norm
                    sum += length(a_m - a_i); // L2 norm
                }
            }
            if(sum < bestSum) {
                bestVector = a_m;
                bestSum = sum;
            }
        }
    }

    write_imagef(output, pos, bestVector.xyyy);
}