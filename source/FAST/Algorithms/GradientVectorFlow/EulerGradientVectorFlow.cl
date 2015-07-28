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
#ifdef cl_khr_3d_image_writes

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

    // The last component of the input vector is stored in f to save memory (f.w)
    f += mu * laplacian - (f - init_vector)*
        (init_vector.x*init_vector.x + init_vector.y*init_vector.y + init_vector.z);

    write_imagef(write_vector_field, writePos, f.xyzz);
}

#else

#endif
