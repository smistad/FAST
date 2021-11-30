const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

inline float2 complexDivision(float2 a, float2 b){
    return (float2)(
        (a.x*b.x + a.y*b.y)/(b.x*b.x + b.y*b.y),
        (a.y*b.x - a.x*b.y)/(b.x*b.x + b.y*b.y)
    );
}

__kernel void envelopeAndLogCompress(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float maxReal,
        __private float maxImaginary
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float2 max = {maxReal, maxImaginary};
    float2 iq = read_imagef(input, sampler, pos).xy;
    write_imagef(output, pos, 20.0f * log10(length(complexDivision(iq, max)))); // Length is abs for complex
}