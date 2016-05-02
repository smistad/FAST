__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;

__kernel void createSegmentation(
        __write_only image2d_t output,
        __private float posX,
        __private float posY,
        __private float radius,
        __private float flattening,
        __private uchar label
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    
    uint result = 0;
    
    float a = radius;
    float b = (1-flattening)*radius;
    float x = pos.x - posX;
    float y = pos.y - posY;
    
    float test = pow(x / a, 2.0f) + pow(y / b, 2.0f);
    
    if(test < 1) {
        result = label;
    }
    
    write_imageui(output, pos, result);
    
}