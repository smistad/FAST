__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
__constant sampler_t hpSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)

#ifdef VECTORS_16BIT
#define FLOAT_TO_SNORM16_4(vector) convert_short4_sat_rte(vector * 32767.0f)
#define FLOAT_TO_SNORM16_3(vector) convert_short3_sat_rte(vector * 32767.0f)
#define FLOAT_TO_SNORM16_2(vector) convert_short2_sat_rte(vector * 32767.0f)
#define FLOAT_TO_SNORM16(vector) convert_short_sat_rte(vector * 32767.0f)
#define VECTOR_FIELD_TYPE short
#else
#define FLOAT_TO_SNORM16_4(vector) vector
#define FLOAT_TO_SNORM16_3(vector) vector
#define FLOAT_TO_SNORM16_2(vector) vector
#define FLOAT_TO_SNORM16(vector) vector
#define VECTOR_FIELD_TYPE float
#endif

__kernel void GVFgaussSeidel(
        __read_only image3d_t r,
        __read_only image3d_t sqrMag,
        __private float mu,
        __private float spacing,
        __read_only image3d_t v_read,
#ifdef fast_3d_image_writes
        __write_only image3d_t v_write
#else
        __global VECTOR_FIELD_TYPE * v_write
#endif
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

    // Calculate manhatten address
    int i = pos.x+pos.y+pos.z;

        // Compute red and put into v_write
        if(i % 2 == 0) {
            float value = native_divide(2.0f*mu*(
                    read_imagef(v_read, sampler, pos + (int4)(1,0,0,0)).x+
                    read_imagef(v_read, sampler, pos - (int4)(1,0,0,0)).x+
                    read_imagef(v_read, sampler, pos + (int4)(0,1,0,0)).x+
                    read_imagef(v_read, sampler, pos - (int4)(0,1,0,0)).x+
                    read_imagef(v_read, sampler, pos + (int4)(0,0,1,0)).x+
                    read_imagef(v_read, sampler, pos - (int4)(0,0,1,0)).x
                    ) - 2.0f*spacing*spacing*read_imagef(r, sampler, pos).x,
                    12.0f*mu+spacing*spacing*read_imagef(sqrMag, sampler, pos).x);
#ifdef fast_3d_image_writes
            write_imagef(v_write, writePos, value);
#else
            v_write[LPOS(writePos)] = FLOAT_TO_SNORM16(value);
#endif
        }
}

__kernel void GVFgaussSeidel2(
        __read_only image3d_t r,
        __read_only image3d_t sqrMag,
        __private float mu,
        __private float spacing,
        __read_only image3d_t v_read,
#ifdef fast_3d_image_writes
        __write_only image3d_t v_write
#else
        __global VECTOR_FIELD_TYPE * v_write
#endif
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

    // Calculate manhatten address
    int i = pos.x+pos.y+pos.z;
	float value;
    if(i % 2 == 0) {
        // Copy red
        value = read_imagef(v_read, sampler, pos).x;
    } else {
            // Compute black
        value = native_divide(2.0f*mu*(
                    read_imagef(v_read, sampler, pos + (int4)(1,0,0,0)).x+
                    read_imagef(v_read, sampler, pos - (int4)(1,0,0,0)).x+
                    read_imagef(v_read, sampler, pos + (int4)(0,1,0,0)).x+
                    read_imagef(v_read, sampler, pos - (int4)(0,1,0,0)).x+
                    read_imagef(v_read, sampler, pos + (int4)(0,0,1,0)).x+
                    read_imagef(v_read, sampler, pos - (int4)(0,0,1,0)).x
                    ) - 2.0f*spacing*spacing*read_imagef(r, sampler, pos).x,
                    12.0f*mu+spacing*spacing*read_imagef(sqrMag, sampler, pos).x);


    }
#ifdef fast_3d_image_writes
	write_imagef(v_write, writePos, value);
#else
	v_write[LPOS(writePos)] = FLOAT_TO_SNORM16(value);
#endif
}


__kernel void addTwoImages(
        __read_only image3d_t i1,
        __read_only image3d_t i2,
#ifdef fast_3d_image_writes
        __write_only image3d_t i3
#else
        __global VECTOR_FIELD_TYPE * i3
#endif
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    float v = read_imagef(i1,sampler,pos).x+read_imagef(i2,sampler,pos).x;
#ifdef fast_3d_image_writes
    write_imagef(i3,pos,v);
#else
    i3[LPOS(pos)] = FLOAT_TO_SNORM16(v);
#endif
}


__kernel void createSqrMag(
        __read_only image3d_t vectorField,
#ifdef fast_3d_image_writes
        __write_only image3d_t sqrMag
#else
        __global VECTOR_FIELD_TYPE * sqrMag
#endif
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    const float4 v = read_imagef(vectorField, sampler, pos);

    float mag = v.x*v.x+v.y*v.y+v.z*v.z;
#ifdef fast_3d_image_writes
    write_imagef(sqrMag, pos, mag);
#else
    sqrMag[LPOS(pos)] = FLOAT_TO_SNORM16(mag);
#endif
}

__kernel void MGGVFInit(
        __read_only image3d_t vectorField,
#ifdef fast_3d_image_writes
        __write_only image3d_t f,
        __write_only image3d_t r,
#else
        __global VECTOR_FIELD_TYPE * f,
        __global VECTOR_FIELD_TYPE * r,
#endif
        __private int component
        ) {

    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    const float4 v = read_imagef(vectorField, sampler, pos);
    const float sqrMag = v.x*v.x+v.y*v.y+v.z*v.z;
    float f_value, r_value;
    if(component == 1) {
        f_value = v.x;
        r_value = -v.x*sqrMag;
    } else if(component == 2) {
        f_value = v.y;
        r_value = -v.y*sqrMag;
    } else {
        f_value = v.z;
        r_value = -v.z*sqrMag;
    }

#ifdef fast_3d_image_writes
    write_imagef(f, pos, f_value);
    write_imagef(r, pos, r_value);
#else
    f[LPOS(pos)] = FLOAT_TO_SNORM16(f_value);
    r[LPOS(pos)] = FLOAT_TO_SNORM16(r_value);
#endif
}

__kernel void MGGVFFinish(
        __read_only image3d_t fx,
        __read_only image3d_t fy,
        __read_only image3d_t fz,
#ifdef fast_3d_image_writes
        __write_only image3d_t vectorField
#else
        //__global VECTOR_FIELD_TYPE * vectorField
        __global float* vectorField
#endif
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    float4 value;
    value.x = read_imagef(fx,sampler,pos).x;
    value.y = read_imagef(fy,sampler,pos).x;
    value.z = read_imagef(fz,sampler,pos).x;
    value.w = length(value.xyz);
#ifdef fast_3d_image_writes
    write_imagef(vectorField,pos,value);
#else
    //vstore4(FLOAT_TO_SNORM16_4(value), LPOS(pos), vectorField);
    vstore4(value, LPOS(pos), vectorField);
#endif
}

__kernel void restrictVolume(
        __read_only image3d_t v_read,
#ifdef fast_3d_image_writes
        __write_only image3d_t v_write
#else
        __global VECTOR_FIELD_TYPE * v_write
#endif
        ) {
        int4 writePos = {
        get_global_id(0),
        get_global_id(1),
        get_global_id(2),
        0
    };
    // Enforce mirror boundary conditions
    int4 size = {get_global_size(0)*2, get_global_size(1)*2, get_global_size(2)*2, 0};
    int4 pos = writePos*2;
    pos = select(pos, size-3, pos >= size-1);

    const int4 readPos = pos;
    const float value = 0.125*(
            read_imagef(v_read, hpSampler, readPos+(int4)(0,0,0,0)).x +
            read_imagef(v_read, hpSampler, readPos+(int4)(1,0,0,0)).x +
            read_imagef(v_read, hpSampler, readPos+(int4)(0,1,0,0)).x +
            read_imagef(v_read, hpSampler, readPos+(int4)(0,0,1,0)).x +
            read_imagef(v_read, hpSampler, readPos+(int4)(1,1,0,0)).x +
            read_imagef(v_read, hpSampler, readPos+(int4)(0,1,1,0)).x +
            read_imagef(v_read, hpSampler, readPos+(int4)(1,1,1,0)).x +
            read_imagef(v_read, hpSampler, readPos+(int4)(1,0,1,0)).x
            );

#ifdef fast_3d_image_writes
    write_imagef(v_write, writePos, value);
#else
    v_write[LPOS(writePos)] = FLOAT_TO_SNORM16(value);
#endif
}

__kernel void prolongate(
        __read_only image3d_t v_l_read,
        __read_only image3d_t v_l_p1,
#ifdef fast_3d_image_writes
        __write_only image3d_t v_l_write
#else
        __global VECTOR_FIELD_TYPE * v_l_write
#endif
        ) {
    const int4 writePos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int4 readPos = convert_int4(floor(convert_float4(writePos)/2.0f));
    float value = read_imagef(v_l_read, hpSampler, writePos).x + read_imagef(v_l_p1, hpSampler, readPos).x;
#ifdef fast_3d_image_writes
    write_imagef(v_l_write, writePos, value);
#else
    v_l_write[LPOS(writePos)] = FLOAT_TO_SNORM16(value);
#endif
}

__kernel void prolongate2(
        __read_only image3d_t v_l_p1,
#ifdef fast_3d_image_writes
        __write_only image3d_t v_l_write
#else
        __global VECTOR_FIELD_TYPE * v_l_write
#endif
        ) {
    const int4 writePos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int4 readPos = convert_int4(floor(convert_float4(writePos)/2.0f));
#ifdef fast_3d_image_writes
    write_imagef(v_l_write, writePos, read_imagef(v_l_p1, hpSampler, readPos).x);
#else
    v_l_write[LPOS(writePos)] = FLOAT_TO_SNORM16(read_imagef(v_l_p1, hpSampler, readPos).x);
#endif
}


__kernel void residual(
        __read_only image3d_t r,
        __read_only image3d_t v,
        __read_only image3d_t sqrMag,
        __private float mu,
        __private float spacing,
#ifdef fast_3d_image_writes
        __write_only image3d_t newResidual
#else
        __global VECTOR_FIELD_TYPE * newResidual
#endif
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

    const float value = read_imagef(r, hpSampler, pos).x -
            ((mu*(
                    read_imagef(v, hpSampler, pos+(int4)(1,0,0,0)).x+
                    read_imagef(v, hpSampler, pos-(int4)(1,0,0,0)).x+
                    read_imagef(v, hpSampler, pos+(int4)(0,1,0,0)).x+
                    read_imagef(v, hpSampler, pos-(int4)(0,1,0,0)).x+
                    read_imagef(v, hpSampler, pos+(int4)(0,0,1,0)).x+
                    read_imagef(v, hpSampler, pos-(int4)(0,0,1,0)).x-
                    6*read_imagef(v, hpSampler, pos).x
                ) / (spacing*spacing))
            - read_imagef(sqrMag, hpSampler, pos).x*read_imagef(v, hpSampler, pos).x);

#ifdef fast_3d_image_writes
    write_imagef(newResidual, writePos, value);
#else
    newResidual[LPOS(writePos)] = FLOAT_TO_SNORM16(value);
#endif
}

__kernel void fmgResidual(
        __read_only image3d_t vectorField,
        __read_only image3d_t v,
        __private float mu,
        __private float spacing,
        __private int component,
#ifdef fast_3d_image_writes
        __write_only image3d_t newResidual
#else
        __global VECTOR_FIELD_TYPE * newResidual
#endif
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

    float4 vector = read_imagef(vectorField, sampler, pos);
    float v0;
    if(component == 1) {
        v0 = vector.x;
    } else if(component == 2) {
       v0 = vector.y;
    } else {
       v0 = vector.z;
    }
    const float sqrMag = vector.x*vector.x+vector.y*vector.y+vector.z*vector.z;

    float residue = (mu*(
                    read_imagef(v, hpSampler, pos+(int4)(1,0,0,0)).x+
                    read_imagef(v, hpSampler, pos-(int4)(1,0,0,0)).x+
                    read_imagef(v, hpSampler, pos+(int4)(0,1,0,0)).x+
                    read_imagef(v, hpSampler, pos-(int4)(0,1,0,0)).x+
                    read_imagef(v, hpSampler, pos+(int4)(0,0,1,0)).x+
                    read_imagef(v, hpSampler, pos-(int4)(0,0,1,0)).x-
                    6*read_imagef(v, hpSampler, pos).x)
                ) / (spacing*spacing);

    //printf("sqrMag: %f, vector value: %f %f %f\n", sqrMag, vector.x, vector.y, vector.z);
    const float value = -sqrMag*v0-(residue - sqrMag*read_imagef(v, hpSampler, pos).x);

#ifdef fast_3d_image_writes
    write_imagef(newResidual, writePos, value);
#else
    newResidual[LPOS(writePos)] = FLOAT_TO_SNORM16(value);
#endif
}


#ifdef fast_3d_image_writes
__kernel void init3DFloat(
        __write_only image3d_t v
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    write_imagef(v, pos, 0.0f);
}
#else
__kernel void initFloatBuffer(
        __global VECTOR_FIELD_TYPE * buffer
        ) {
    buffer[get_global_id(0)] = FLOAT_TO_SNORM16(0.0f);
}
#endif
