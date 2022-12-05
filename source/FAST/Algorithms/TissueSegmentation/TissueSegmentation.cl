__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void segment(
            __read_only image2d_t input,
            __write_only image2d_t output,
            __private int m_thresh,
            __private int m_filterZeros
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    float3 color = convert_float4(read_imageui(input, sampler, pos)).xyz;

    uchar result = 0;
    if ((!(m_filterZeros && all(color == 0))) && (length(color - (float3)(255, 255, 255)) > m_thresh)) {
        result = 1;
    }

    write_imageui(output, pos, result);
}