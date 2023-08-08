const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

inline float2 complexDivision(float2 a, float2 b){
    return (float2)(
        (a.x*b.x + a.y*b.y)/(b.x*b.x + b.y*b.y),
        (a.y*b.x - a.x*b.y)/(b.x*b.x + b.y*b.y)
    );
}

__kernel void envelopeAndLogCompress(
        __read_only image2d_t input,
        __write_only image2d_t output
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float2 iq = read_imagef(input, sampler, pos).xy;
    write_imagef(output, pos, 20.0f * log10(length(iq))); // Length is abs for complex
}

__kernel void normalizeEnvelope(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float maxdB
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    write_imagef(output, pos, read_imagef(input, sampler, pos).x - maxdB);
}

__kernel void normalizeEnvelopeToGrayscale(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float maxdB,
        __private float gain,
        __private float dynamicRange
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float dBPixel = read_imagef(input, sampler, pos).x - maxdB;
    float img_sc_reject = dBPixel + gain;
    img_sc_reject = (img_sc_reject < -dynamicRange) ? -dynamicRange : img_sc_reject; //Reject everything below dynamic range
    img_sc_reject = (img_sc_reject > 0) ? 0 : img_sc_reject; // Everything above 0 dB should be saturated
    uchar img_gray_scale = round(255*(img_sc_reject+dynamicRange)/dynamicRange);
    write_imageui(output, pos, img_gray_scale);
}
