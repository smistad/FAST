__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void GVF2DCopy(__read_only image2d_t input, __write_only image2d_t output) {
    int2 pos = {get_global_id(0), get_global_id(1)};
    write_imagef(output, pos, read_imagef(input, sampler, pos)); 
}

__kernel void GVF2DIteration(__read_only image2d_t init_vector_field, __read_only image2d_t read_vector_field, __write_only image2d_t write_vector_field, __private float mu) {

    int2 writePos = {get_global_id(0), get_global_id(1)};

    // Enforce mirror boundary conditions
    int2 size = {get_global_size(0), get_global_size(1)};
    int2 pos = writePos;
    pos = select(pos, (int2)(2,2), pos == (int2)(0,0));
    pos = select(pos, size-3, pos >= size-1);

    float2 f = read_imagef(read_vector_field, sampler, pos).xy;

    const float2 init_vector = read_imagef(init_vector_field, sampler, pos).xy;

    const float2 fx1 = read_imagef(read_vector_field, sampler, pos + (int2)(1,0)).xy;
    const float2 fy1 = read_imagef(read_vector_field, sampler, pos + (int2)(0,1)).xy;
    const float2 fx_1 = read_imagef(read_vector_field, sampler, pos - (int2)(1,0)).xy;
    const float2 fy_1 = read_imagef(read_vector_field, sampler, pos - (int2)(0,1)).xy;

    // Update the vector field: Calculate Laplacian using a 3D central difference scheme
    float2 laplacian = -4*f + fx1 + fx_1 + fy1 + fy_1;

    f += mu * laplacian - (f - init_vector)*(init_vector.x*init_vector.x + init_vector.y*init_vector.y);

    write_imagef(write_vector_field, writePos, (float4)(f.x,f.y,0,0));
}


// If device supports writing to 3D textures
#ifdef fast_3d_image_writes

__kernel void GVF3DCopy(__read_only image3d_t input, __write_only image3d_t output) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    write_imagef(output, pos, read_imagef(input, sampler, pos)); 
}

__kernel void GVF3DIteration(
        __read_only image3d_t init_vector_field, 
        __read_only image3d_t read_vector_field, 
        __write_only image3d_t write_vector_field, 
        __private float mu
    ) {

    int4 writePos = {
        get_global_id(0),
        get_global_id(1),
        get_global_id(2),
        0
    };
    // Enforce mirror boundary conditions
    int4 size = {get_image_width(init_vector_field), get_image_height(init_vector_field), get_image_depth(init_vector_field), 0};
    int4 pos = writePos;
    pos = select(pos, (int4)(2,2,2,0), pos == (int4)(0,0,0,0));
    pos = select(pos, size-3, pos >= size-1);

    // Load data from memory and do calculations
    float3 init_vector = read_imagef(init_vector_field, sampler, pos).xyz;

    float3 f = read_imagef(read_vector_field, sampler, pos).xyz;
    float3 fx1 = read_imagef(read_vector_field, sampler, pos + (int4)(1,0,0,0)).xyz; 
    float3 fx_1 = read_imagef(read_vector_field, sampler, pos - (int4)(1,0,0,0)).xyz; 
    float3 fy1 = read_imagef(read_vector_field, sampler, pos + (int4)(0,1,0,0)).xyz; 
    float3 fy_1 = read_imagef(read_vector_field, sampler, pos - (int4)(0,1,0,0)).xyz; 
    float3 fz1 = read_imagef(read_vector_field, sampler, pos + (int4)(0,0,1,0)).xyz; 
    float3 fz_1 = read_imagef(read_vector_field, sampler, pos - (int4)(0,0,1,0)).xyz; 
    
    // Update the vector field: Calculate Laplacian using a 3D central difference scheme
    float3 laplacian = -6*f.xyz + fx1 + fx_1 + fy1 + fy_1 + fz1 + fz_1;

    // NOT ANYMORE: The last component of the input vector is stored in f to save memory (f.w)
    f += mu * laplacian - (f - init_vector)*
        (init_vector.x*init_vector.x + init_vector.y*init_vector.y + init_vector.z*init_vector.z);

    write_imagef(write_vector_field, writePos, f.xyzz);
}

#else

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)
#ifdef VECTORS_16BIT
#define FLOAT_TO_SNORM16_4(vector) convert_short4_sat_rte(vector * 32767.0f)
#define SNORM16_TO_FLOAT_4(vector) max(-1.0f, convert_float4(vector) / 32767.0f)
#define FLOAT_TO_SNORM16_3(vector) convert_short3_sat_rte(vector * 32767.0f)
#define SNORM16_TO_FLOAT_3(vector) max(-1.0f, convert_float3(vector) / 32767.0f)
#define FLOAT_TO_SNORM16_2(vector) convert_short2_sat_rte(vector * 32767.0f)
#define SNORM16_TO_FLOAT_2(vector) max(-1.0f, convert_float2(vector) / 32767.0f)
#define FLOAT_TO_SNORM16(vector) convert_short_sat_rte(vector * 32767.0f)
#define SNORM16_TO_FLOAT(vector) max(-1.0f, convert_float(vector) / 32767.0f)
#define VECTOR_FIELD_TYPE short
#else
#define FLOAT_TO_SNORM16_4(vector) vector
#define SNORM16_TO_FLOAT_4(vector) vector
#define FLOAT_TO_SNORM16_3(vector) vector
#define SNORM16_TO_FLOAT_3(vector) vector
#define FLOAT_TO_SNORM16_2(vector) vector
#define SNORM16_TO_FLOAT_2(vector) vector
#define FLOAT_TO_SNORM16(vector) vector
#define SNORM16_TO_FLOAT(vector) vector
#define VECTOR_FIELD_TYPE float
#endif

__kernel void GVF3DIteration(
        __read_only image3d_t init_vector_field,
        __global VECTOR_FIELD_TYPE const * restrict read_vector_field,
        __global VECTOR_FIELD_TYPE * write_vector_field,
        __private float mu
        ) {
    int4 writePos = {
        get_global_id(0),
        get_global_id(1),
        get_global_id(2),
        0
    };
    // Enforce mirror boundary conditions
    int4 size = {get_global_size(0), get_global_size(1), get_global_size(2), 0};
    int4 pos = writePos;
    pos = select(pos, (int4)(2,2,2,0), pos == (int4)(0,0,0,0));
    pos = select(pos, size-3, pos >= size-1);
    int offset = pos.x + pos.y*size.x + pos.z*size.x*size.y;

    // Load data from shared memory and do calculations
    float4 init_vector = read_imagef(init_vector_field, sampler, pos);

    float3 v = SNORM16_TO_FLOAT_3(vload3(offset, read_vector_field));
    float3 fx1 = SNORM16_TO_FLOAT_3(vload3(offset+1, read_vector_field));
    float3 fx_1 = SNORM16_TO_FLOAT_3(vload3(offset-1, read_vector_field));
    float3 fy1 = SNORM16_TO_FLOAT_3(vload3(offset+size.x, read_vector_field));
    float3 fy_1 = SNORM16_TO_FLOAT_3(vload3(offset-size.x, read_vector_field));
    float3 fz1 = SNORM16_TO_FLOAT_3(vload3(offset+size.x*size.y, read_vector_field));
    float3 fz_1 = SNORM16_TO_FLOAT_3(vload3(offset-size.x*size.y, read_vector_field));
    
    // Update the vector field: Calculate Laplacian using a 3D central difference scheme
    float3 v2;
    v2.x = -6*v.x;
    v2.y = -6*v.y;
    v2.z = -6*v.z;
    float3 laplacian = v2 + fx1 + fx_1 + fy1 + fy_1 + fz1 + fz_1;

    v += mu*laplacian - (v - init_vector.xyz)*(init_vector.x*init_vector.x + init_vector.y*init_vector.y + init_vector.z*init_vector.z);

    vstore3(FLOAT_TO_SNORM16_3(v), writePos.x + writePos.y*size.x + writePos.z*size.x*size.y, write_vector_field);
}


__kernel void GVF3DInit(
        __read_only image3d_t vectorFieldImage,
        __global VECTOR_FIELD_TYPE * vectorField
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    vstore3(FLOAT_TO_SNORM16_3(read_imagef(vectorFieldImage, sampler, pos).xyz), LPOS(pos), vectorField);
}

__kernel void GVF3DFinish(
        __global VECTOR_FIELD_TYPE * vectorField,
        //__global VECTOR_FIELD_TYPE * vectorField2
        __global float * vectorField2
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    float4 v;
    v.xyz = SNORM16_TO_FLOAT_3(vload3(LPOS(pos), vectorField));
    v.w = 0;
    v.w = length(v) > 0.0f ? length(v) : 1.0f;
    //vstore4(FLOAT_TO_SNORM16_4(v), LPOS(pos), vectorField2);
    vstore4(v, LPOS(pos), vectorField2);
}

#endif
