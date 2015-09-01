__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
__constant sampler_t hpSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void GVFgaussSeidel(
        __read_only image3d_t r,
        __read_only image3d_t sqrMag,
        __private float mu,
        __private float spacing,
        __read_only image3d_t v_read,
        __write_only image3d_t v_write
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
            write_imagef(v_write, writePos, value);
        }
}

__kernel void GVFgaussSeidel2(
        __read_only image3d_t r,
        __read_only image3d_t sqrMag,
        __private float mu,
        __private float spacing,
        __read_only image3d_t v_read,
        __write_only image3d_t v_write
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

        if(i % 2 == 0) {
            // Copy red
            float value = read_imagef(v_read, sampler, pos).x;
        write_imagef(v_write, writePos, value);
        } else {
            // Compute black
                float value = native_divide(2.0f*mu*(
                    read_imagef(v_read, sampler, pos + (int4)(1,0,0,0)).x+
                    read_imagef(v_read, sampler, pos - (int4)(1,0,0,0)).x+
                    read_imagef(v_read, sampler, pos + (int4)(0,1,0,0)).x+
                    read_imagef(v_read, sampler, pos - (int4)(0,1,0,0)).x+
                    read_imagef(v_read, sampler, pos + (int4)(0,0,1,0)).x+
                    read_imagef(v_read, sampler, pos - (int4)(0,0,1,0)).x
                    ) - 2.0f*spacing*spacing*read_imagef(r, sampler, pos).x,
                    12.0f*mu+spacing*spacing*read_imagef(sqrMag, sampler, pos).x);

        write_imagef(v_write, writePos, value);
        }
}


__kernel void addTwoImages(
        __read_only image3d_t i1,
        __read_only image3d_t i2,
        __write_only image3d_t i3
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    float v = read_imagef(i1,sampler,pos).x+read_imagef(i2,sampler,pos).x;
    write_imagef(i3,pos,v);
}


__kernel void createSqrMag(
        __read_only image3d_t vectorField,
        __write_only image3d_t sqrMag
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    const float4 v = read_imagef(vectorField, sampler, pos);

    float mag = v.x*v.x+v.y*v.y+v.z*v.z;
    write_imagef(sqrMag, pos, mag);
}

__kernel void MGGVFInit(
        __read_only image3d_t vectorField,
        __write_only image3d_t f,
        __write_only image3d_t r,
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

    write_imagef(f, pos, f_value);
    write_imagef(r, pos, r_value);
}

__kernel void MGGVFFinish(
        __read_only image3d_t fx,
        __read_only image3d_t fy,
        __read_only image3d_t fz,
        __write_only image3d_t vectorField
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    float4 value;
    value.x = read_imagef(fx,sampler,pos).x;
    value.y = read_imagef(fy,sampler,pos).x;
    value.z = read_imagef(fz,sampler,pos).x;
    value.w = length(value.xyz);
    write_imagef(vectorField,pos,value);
}

__kernel void restrictVolume(
        __read_only image3d_t v_read,
        __write_only image3d_t v_write
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

    write_imagef(v_write, writePos, value);
}

__kernel void prolongate(
        __read_only image3d_t v_l_read,
        __read_only image3d_t v_l_p1,
        __write_only image3d_t v_l_write
        ) {
    const int4 writePos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int4 readPos = convert_int4(floor(convert_float4(writePos)/2.0f));
    write_imagef(v_l_write, writePos, read_imagef(v_l_read, hpSampler, writePos).x + read_imagef(v_l_p1, hpSampler, readPos).x);
}

__kernel void prolongate2(
        __read_only image3d_t v_l_p1,
        __write_only image3d_t v_l_write
        ) {
    const int4 writePos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int4 readPos = convert_int4(floor(convert_float4(writePos)/2.0f));
    write_imagef(v_l_write, writePos, read_imagef(v_l_p1, hpSampler, readPos).x);
}


__kernel void residual(
        __read_only image3d_t r,
        __read_only image3d_t v,
        __read_only image3d_t sqrMag,
        __private float mu,
        __private float spacing,
        __write_only image3d_t newResidual
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

    write_imagef(newResidual, writePos, value);
}

__kernel void fmgResidual(
        __read_only image3d_t vectorField,
        __read_only image3d_t v,
        __private float mu,
        __private float spacing,
        __private int component,
        __write_only image3d_t newResidual
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

    write_imagef(newResidual, writePos, value);
}


__kernel void init3DFloat(
        __write_only image3d_t v
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    write_imagef(v, pos, 0.0f);
}
