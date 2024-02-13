__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;


float readImageAsFloat2D(__read_only image2d_t image, sampler_t sampler, int2 position) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        return read_imagef(image, sampler, position).x;
    } else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT32) {
        return (float)read_imagei(image, sampler, position).x;
    } else {
        return (float)read_imageui(image, sampler, position).x;
    }
}

float readImageAsFloat3D(__read_only image3d_t image, sampler_t sampler, int4 position) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        return read_imagef(image, sampler, position).x;
    } else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT32) {
        return (float)read_imagei(image, sampler, position).x;
    } else {
        return (float)read_imageui(image, sampler, position).x;
    }
}

__kernel void gradient2D(
        __read_only image2d_t input,
        __write_only image2d_t output
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    
    // TODO take pixel spacing into account
    float2 gradient = {
            (readImageAsFloat2D(input, sampler, pos+(int2)(1,0)) - readImageAsFloat2D(input, sampler, pos-(int2)(1,0)))*0.5f,
            (readImageAsFloat2D(input, sampler, pos+(int2)(0,1)) - readImageAsFloat2D(input, sampler, pos-(int2)(0,1)))*0.5f
    };
    write_imagef(output, pos, gradient.xyyy);
}


#ifdef VECTORS_16BIT
#define FLOAT_TO_SNORM16_3(vector) convert_short3_sat_rte(vector * 32767.0f)
#define VECTOR_FIELD_TYPE short
#else
#define FLOAT_TO_SNORM16_3(vector) vector
#define VECTOR_FIELD_TYPE float
#endif

__kernel void gradient3D(
        __read_only image3d_t input,
#ifdef fast_3d_image_writes
        __write_only image3d_t output
#else
        __global VECTOR_FIELD_TYPE* output
#endif
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    
    // TODO take pixel spacing into account
    float3 gradient = {
            (readImageAsFloat3D(input, sampler, pos+(int4)(1,0,0,0)) - readImageAsFloat3D(input, sampler, pos-(int4)(1,0,0,0)))*0.5f,
            (readImageAsFloat3D(input, sampler, pos+(int4)(0,1,0,0)) - readImageAsFloat3D(input, sampler, pos-(int4)(0,1,0,0)))*0.5f,
            (readImageAsFloat3D(input, sampler, pos+(int4)(0,0,1,0)) - readImageAsFloat3D(input, sampler, pos-(int4)(0,0,1,0)))*0.5f
    };
#ifdef fast_3d_image_writes
    write_imagef(output, pos, gradient.xyzz);
#else 
    vstore3(FLOAT_TO_SNORM16_3(gradient), pos.x + pos.y*get_global_size(0) + pos.z*get_global_size(0)*get_global_size(1), output);
#endif
}

