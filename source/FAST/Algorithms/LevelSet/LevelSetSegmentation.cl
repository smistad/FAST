#ifdef NO_3D_WRITE
#define PHI_WRITE_TYPE __global float *
#define WRITE_RESULT(storage, pos, value) storage[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = value;
#else
#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable
#define PHI_WRITE_TYPE __write_only image3d_t
#define WRITE_RESULT(storage, pos, value) write_imagef(storage, pos, value)
#endif

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void updateLevelSetFunction(
        __read_only image3d_t input,
        __read_only image3d_t phi_read,
        PHI_WRITE_TYPE phi_write,
        __private float threshold,
        __private float epsilon,
        __private float alpha,
        PHI_WRITE_TYPE speedStorage,
        __private float deltaT
) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    int z = get_global_id(2);
    const int4 pos = {x,y,z,0};

    // Calculate all first order derivatives
    float3 D = {
            0.5f*(read_imagef(phi_read,sampler,(int4)(x+1,y,z,0)).x-read_imagef(phi_read,sampler,(int4)(x-1,y,z,0)).x),
            0.5f*(read_imagef(phi_read,sampler,(int4)(x,y+1,z,0)).x-read_imagef(phi_read,sampler,(int4)(x,y-1,z,0)).x),
            0.5f*(read_imagef(phi_read,sampler,(int4)(x,y,z+1,0)).x-read_imagef(phi_read,sampler,(int4)(x,y,z-1,0)).x)
    };
    float3 Dminus = {
            read_imagef(phi_read,sampler,pos).x-read_imagef(phi_read,sampler,(int4)(x-1,y,z,0)).x,
            read_imagef(phi_read,sampler,pos).x-read_imagef(phi_read,sampler,(int4)(x,y-1,z,0)).x,
            read_imagef(phi_read,sampler,pos).x-read_imagef(phi_read,sampler,(int4)(x,y,z-1,0)).x
    };
    float3 Dplus = {
            read_imagef(phi_read,sampler,(int4)(x+1,y,z,0)).x-read_imagef(phi_read,sampler,pos).x,
            read_imagef(phi_read,sampler,(int4)(x,y+1,z,0)).x-read_imagef(phi_read,sampler,pos).x,
            read_imagef(phi_read,sampler,(int4)(x,y,z+1,0)).x-read_imagef(phi_read,sampler,pos).x
    };

    // Calculate gradient
    float3 gradientMin = {
            sqrt(pow(min(Dplus.x, 0.0f), 2.0f) + pow(min(-Dminus.x, 0.0f), 2.0f)),
            sqrt(pow(min(Dplus.y, 0.0f), 2.0f) + pow(min(-Dminus.y, 0.0f), 2.0f)),
            sqrt(pow(min(Dplus.z, 0.0f), 2.0f) + pow(min(-Dminus.z, 0.0f), 2.0f))
    };
    float3 gradientMax = {
            sqrt(pow(max(Dplus.x, 0.0f), 2.0f) + pow(max(-Dminus.x, 0.0f), 2.0f)),
            sqrt(pow(max(Dplus.y, 0.0f), 2.0f) + pow(max(-Dminus.y, 0.0f), 2.0f)),
            sqrt(pow(max(Dplus.z, 0.0f), 2.0f) + pow(max(-Dminus.z, 0.0f), 2.0f))
    };

    // Calculate all second order derivatives
    float3 DxMinus = {
            0.0f,
            0.5f*(read_imagef(phi_read,sampler,(int4)(x+1,y-1,z,0)).x-read_imagef(phi_read,sampler,(int4)(x-1,y-1,z,0)).x),
            0.5f*(read_imagef(phi_read,sampler,(int4)(x+1,y,z-1,0)).x-read_imagef(phi_read,sampler,(int4)(x-1,y,z-1,0)).x)
    };
    float3 DxPlus = {
            0.0f,
            0.5f*(read_imagef(phi_read,sampler,(int4)(x+1,y+1,z,0)).x-read_imagef(phi_read,sampler,(int4)(x-1,y+1,z,0)).x),
            0.5f*(read_imagef(phi_read,sampler,(int4)(x+1,y,z+1,0)).x-read_imagef(phi_read,sampler,(int4)(x-1,y,z+1,0)).x)
    };
    float3 DyMinus = {
            0.5f*(read_imagef(phi_read,sampler,(int4)(x-1,y+1,z,0)).x-read_imagef(phi_read,sampler,(int4)(x-1,y-1,z,0)).x),
            0.0f,
            0.5f*(read_imagef(phi_read,sampler,(int4)(x,y+1,z-1,0)).x-read_imagef(phi_read,sampler,(int4)(x,y-1,z-1,0)).x)
    };
    float3 DyPlus = {
            0.5f*(read_imagef(phi_read,sampler,(int4)(x+1,y+1,z,0)).x-read_imagef(phi_read,sampler,(int4)(x+1,y-1,z,0)).x),
            0.0f,
            0.5f*(read_imagef(phi_read,sampler,(int4)(x,y+1,z+1,0)).x-read_imagef(phi_read,sampler,(int4)(x,y-1,z+1,0)).x)
    };
    float3 DzMinus = {
            0.5f*(read_imagef(phi_read,sampler,(int4)(x-1,y,z+1,0)).x-read_imagef(phi_read,sampler,(int4)(x-1,y,z-1,0)).x),
            0.5f*(read_imagef(phi_read,sampler,(int4)(x,y-1,z+1,0)).x-read_imagef(phi_read,sampler,(int4)(x,y-1,z-1,0)).x),
            0.0f
    };
    float3 DzPlus = {
            0.5f*(read_imagef(phi_read,sampler,(int4)(x+1,y,z+1,0)).x-read_imagef(phi_read,sampler,(int4)(x+1,y,z-1,0)).x),
            0.5f*(read_imagef(phi_read,sampler,(int4)(x,y+1,z+1,0)).x-read_imagef(phi_read,sampler,(int4)(x,y+1,z-1,0)).x),
            0.0f
    };

    // Calculate curvature
    float3 nMinus = {
            Dminus.x / sqrt(FLT_EPSILON+Dminus.x*Dminus.x+pow(0.5f*(DyMinus.x+D.y),2.0f)+pow(0.5f*(DzMinus.x+D.z),2.0f)),
            Dminus.y / sqrt(FLT_EPSILON+Dminus.y*Dminus.y+pow(0.5f*(DxMinus.y+D.x),2.0f)+pow(0.5f*(DzMinus.y+D.z),2.0f)),
            Dminus.z / sqrt(FLT_EPSILON+Dminus.z*Dminus.z+pow(0.5f*(DxMinus.z+D.x),2.0f)+pow(0.5f*(DyMinus.z+D.y),2.0f))
    };
    float3 nPlus = {
            Dplus.x / sqrt(FLT_EPSILON+Dplus.x*Dplus.x+pow(0.5f*(DyPlus.x+D.y),2.0f)+pow(0.5f*(DzPlus.x+D.z),2.0f)),
            Dplus.y / sqrt(FLT_EPSILON+Dplus.y*Dplus.y+pow(0.5f*(DxPlus.y+D.x),2.0f)+pow(0.5f*(DzPlus.y+D.z),2.0f)),
            Dplus.z / sqrt(FLT_EPSILON+Dplus.z*Dplus.z+pow(0.5f*(DxPlus.z+D.x),2.0f)+pow(0.5f*(DyPlus.z+D.y),2.0f))
    };

    float curvature = ((nPlus.x-nMinus.x)+(nPlus.y-nMinus.y)+(nPlus.z-nMinus.z))*0.5f;

    // Calculate speed term
    float speed = -(1.0f-alpha)*max(-epsilon, (epsilon-fabs(threshold-read_imagei(input,sampler,pos).x)))/epsilon + alpha*curvature;

    // Determine gradient based on speed direction
    float3 gradient;
    if(speed < 0) {
        gradient = gradientMin;
    } else {
        gradient = gradientMax;
    }
    if(length(gradient) > 1.0f)
        gradient = normalize(gradient);

    // Stability CFL
    // max(fabs(speed*gradient.length()))
    WRITE_RESULT(speedStorage, pos, fabs(speed*length(gradient)));

    // Update the level set function phi
    WRITE_RESULT(phi_write, pos, read_imagef(phi_read,sampler,pos).x + deltaT*speed*length(gradient));
}

__kernel void initializeLevelSetFunction(
        PHI_WRITE_TYPE phi,
        __private int seedX,
        __private int seedY,
        __private int seedZ,
        __private float radius
) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    WRITE_RESULT(phi, pos, distance((float3)(seedX,seedY,seedZ), convert_float3(pos.xyz)) - radius);
}