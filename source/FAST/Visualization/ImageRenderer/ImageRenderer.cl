__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

float4 readPixelAsFloat(image2d_t image, sampler_t sampler, int2 pos) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT) {
        return read_imagef(image, sampler, pos);
    } else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16) {
        return convert_float4(read_imagei(image, sampler, pos));
    } else {
        return convert_float4(read_imageui(image, sampler, pos));
    }
}

__kernel void renderToTexture(
        __read_only image2d_t image,
        __write_only image2d_t texture,
        __private float level,
        __private float window
        ) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    float4 value = readPixelAsFloat(image, sampler, (int2)(x,y));
    if(get_image_channel_order(image) == CLK_R) {
        value.y = value.x;
        value.z = value.x;
    }

    value = (value - level + window/2) / window;
    value = clamp(value, 0.0f, 1.0f);
    value.w = 1.0f;
    write_imagef(texture, (int2)(x,get_image_height(image) - y - 1), value);
}
