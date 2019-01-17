

__kernel void volumeRender(
        __read_only image3d_t volume,
        __write_only image2d_t framebuffer
        ) {
    int2 position = {get_global_id(0), get_global_id(1)};
    float4 value = {0.0f, 1.0f, 0.0f, 1.0f};
    write_imagef(framebuffer, position, value);
}