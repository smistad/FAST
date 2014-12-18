
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

#ifdef TYPE_UINT
#define READ_IMAGE (float)read_imageui
#elif TYPE_INT
#define READ_IMAGE (float)read_imagei
#elif TYPE_FLOAT
#define READ_IMAGE read_imagef
#endif

__kernel void gradient2D(
        __read_only image2d_t input,
        __write_only image2d_t output
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    
    // TODO take pixel spacing into account
    float4 gradient = {
            (READ_IMAGE(input, sampler, pos+(int2)(1,0)).x - READ_IMAGE(input, sampler, pos-(int2)(1,0)).x)/2.0f,
            (READ_IMAGE(input, sampler, pos+(int2)(0,1)).x - READ_IMAGE(input, sampler, pos-(int2)(0,1)).x)/2.0f,
            0,0
    };
    write_imagef(output, pos, gradient);
}

#ifdef cl_khr_3d_image_writes
__kernel void gradient3D(
        __read_only image3d_t input,
        __write_only image3d_t output
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    
    // TODO take pixel spacing into account
    float4 gradient = {
            (READ_IMAGE(input, sampler, pos+(int4)(1,0,0,0)).x - READ_IMAGE(input, sampler, pos-(int4)(1,0,0,0)).x)*0.5f,
            (READ_IMAGE(input, sampler, pos+(int4)(0,1,0,0)).x - READ_IMAGE(input, sampler, pos-(int4)(0,1,0,0)).x)*0.5f,
            (READ_IMAGE(input, sampler, pos+(int4)(0,0,1,0)).x - READ_IMAGE(input, sampler, pos-(int4)(0,0,1,0)).x)*0.5f,
            0
    };
    write_imagef(output, pos, gradient);
}
#else // cl_khr_3d_image_writes
__kernel void gradient3D(
        __read_only image3d_t input,
        __global float* output
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    
    // TODO take pixel spacing into account
    float4 gradient = {
            (READ_IMAGE(input, sampler, pos+(int4)(1,0,0,0)).x - READ_IMAGE(input, sampler, pos-(int4)(1,0,0,0)).x)*0.5f,
            (READ_IMAGE(input, sampler, pos+(int4)(0,1,0,0)).x - READ_IMAGE(input, sampler, pos-(int4)(0,1,0,0)).x)*0.5f,
            (READ_IMAGE(input, sampler, pos+(int4)(0,0,1,0)).x - READ_IMAGE(input, sampler, pos-(int4)(0,0,1,0)).x)*0.5f,
            0
    };
    vstore3(gradient, pos.x()+pos.y()*get_global_size(0)+pos.z()*get_global_size(0)*get_global_size(1), output);
}
#endif // cl_khr_3d_image_writes

