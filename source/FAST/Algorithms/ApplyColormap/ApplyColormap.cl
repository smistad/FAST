__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

float4 readImageAsFloat2D(__read_only image2d_t image, sampler_t sampler, int2 position) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        return read_imagef(image, sampler, position);
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        return convert_float4(read_imagei(image, sampler, position));
    } else {
        return convert_float4(read_imageui(image, sampler, position));
    }
}


void writeImageAsFloat42D(__write_only image2d_t image, int2 position, float4 value) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        write_imagef(image, position, value);
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        write_imagei(image, position, convert_int4(round(value)));
    } else {
        write_imageui(image, position, convert_uint4(round(value)));
    }
}


void writeImageAsFloat2D(__write_only image2d_t image, int2 position, float value) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        write_imagef(image, position, value);
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        write_imagei(image, position, convert_int4(round(value)));
    } else {
        write_imageui(image, position, convert_uint4(round(value)));
    }
}

float4 vload4_at_pos(int position, __constant float* data) {
    return (float4)(data[position], data[position+1], data[position+2], 1.0f);
}

float getIntensityFromColormap(float intensity, __constant float* colormap, int steps, char interpolate) {
    float first = colormap[1];
    float firstIntensity = colormap[0];

    if(intensity <= firstIntensity)
        return first;

    for(int i = 1; i < steps; ++i) {
        float second = colormap[i*2+1];
        float secondIntensity = colormap[i*2];
        if(intensity <= secondIntensity) {
            if(interpolate == 1) {
                return mix(first, second, (intensity - firstIntensity)/(secondIntensity - firstIntensity));
            } else {
                // Nearest
                if(intensity - firstIntensity < secondIntensity - intensity) {
                    return first;
                } else {
                    return second;
                }
            }
        }
        first = second;
        firstIntensity = secondIntensity;
    }
    return first;
}

float4 getColorFromColormap(float intensity, __constant float* colormap, int steps, char interpolate) {
    float4 first = vload4_at_pos(1, colormap);
    float firstIntensity = colormap[0];

    if(intensity <= firstIntensity)
        return first;

    for(int i = 1; i < steps; ++i) {
        float4 second = vload4_at_pos(i*4 + 1, colormap);
        float secondIntensity = colormap[i*4];
        if(intensity <= secondIntensity) {
            if(interpolate == 1) {
                return mix(first, second, (intensity - firstIntensity)/(secondIntensity - firstIntensity));
            } else {
                // Nearest
                if(intensity - firstIntensity < secondIntensity - intensity) {
                    return first;
                } else {
                    return second;
                }
            }
        }
        first = second;
        firstIntensity = secondIntensity;
    }
    return first;
}

__kernel void applyColormapGrayscale(
            __read_only image2d_t input,
            __write_only image2d_t output,
            __constant float* colormap,
            __private int steps,
            __private char interpolate,
            __private char grayscale
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float4 value = readImageAsFloat2D(input, sampler, pos);

    if(grayscale == 1) {
        writeImageAsFloat2D(output, pos, getIntensityFromColormap(value.x, colormap, steps, interpolate));
    } else {
        writeImageAsFloat42D(output, pos, getColorFromColormap(value.x, colormap, steps, interpolate));
    }
}