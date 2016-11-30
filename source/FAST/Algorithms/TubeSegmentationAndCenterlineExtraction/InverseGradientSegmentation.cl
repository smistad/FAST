__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)

__kernel void initGrowing(
        __read_only image3d_t centerline,
#ifdef fast_3d_image_writes
        __write_only image3d_t initSegmentation
#else
    __global uchar * initSegmentation
#endif
    ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    if(read_imageui(centerline, sampler, pos).x > 0) {
        uint radius = read_imageui(centerline, sampler, pos).x;
        int N;
        if(radius > 7) {
            N = min(max(1, (int)(radius)), 5);
        } else {
            N = 1;
        }

        for(int a = -N; a < N+1; a++) {
        for(int b = -N; b < N+1; b++) {
        for(int c = -N; c < N+1; c++) {
            int4 n;
            n.x = pos.x + a;
            n.y = pos.y + b;
            n.z = pos.z + c;
            if(read_imageui(centerline, sampler, n).x == 0 && length((float3)(a,b,c)) <= N) {
#ifdef fast_3d_image_writes
                write_imageui(initSegmentation, n, 2);
#else
                if(n.x >= 0 && n.y >= 0 && n.z >= 0 &&
                    n.x < get_global_size(0) && n.y < get_global_size(1) && n.z < get_global_size(2))
                    initSegmentation[LPOS(n)] = 2; 
#endif
        }
        }}}
    }

}

__kernel void grow(
        __read_only image3d_t currentSegmentation,
        __read_only image3d_t gvf,
#ifdef fast_3d_image_writes
        __write_only image3d_t nextSegmentation,
#else
        __global uchar* nextSegmentation,
#endif
        __global int * stop
    ) {

    int4 X = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    char value = read_imageui(currentSegmentation, sampler, X).x;
    // value of 2, means to check it, 1 means it is already accepted
    if(value == 1) {
#ifdef fast_3d_image_writes
        write_imageui(nextSegmentation, X, 1);
#else
        nextSegmentation[LPOS(X)] = 1;
#endif
    } else if(value == 2){
        float4 FNX = read_imagef(gvf, sampler, X);
        float FNXw = length(FNX.xyz);

        bool continueGrowing = false;
        for(int a = -1; a < 2; a++) {
        for(int b = -1; b < 2; b++) {
        for(int c = -1; c < 2; c++) {
            if(a == 0 && b == 0 && c == 0)
                continue;

            int4 Y;
            Y.x = X.x + a;
            Y.y = X.y + b;
            Y.z = X.z + c;
            
            char valueY = read_imageui(currentSegmentation, sampler, Y).x;
            if(valueY != 1) {
                float4 FNY = read_imagef(gvf, sampler, Y);
                FNY.w = length(FNY.xyz);
                FNY.x /= FNY.w;
                FNY.y /= FNY.w;
                FNY.z /= FNY.w;
                if(FNY.w > FNXw /*|| FNXw < 0.1f*/) {
                    int4 Z;
                    float maxDotProduct = -2.0f;
                    for(int a2 = -1; a2 < 2; a2++) {
                    for(int b2 = -1; b2 < 2; b2++) {
                    for(int c2 = -1; c2 < 2; c2++) {
                        if(a2 == 0 && b2 == 0 && c2 == 0)
                            continue;
                        int4 Zc;
                        Zc.x = Y.x+a2;
                        Zc.y = Y.y+b2;
                        Zc.z = Y.z+c2;
                        float3 YZ;
                        YZ.x = Zc.x-Y.x;
                        YZ.y = Zc.y-Y.y;
                        YZ.z = Zc.z-Y.z;
                        YZ = normalize(YZ);
                        if(dot(FNY.xyz, YZ) > maxDotProduct) {
                            maxDotProduct = dot(FNY.xyz, YZ);
                            Z = Zc;
                        }
                    }}}

                    if(Z.x == X.x && Z.y == X.y && Z.z == X.z) {
#ifdef fast_3d_image_writes
                        write_imageui(nextSegmentation, X, 1);
                        write_imageui(nextSegmentation, Y, 2);
#else
                        nextSegmentation[LPOS(X)] = 1;
                        // Check if in bounds
                        if(Y.x >= 0 && Y.y >= 0 && Y.z >= 0 && 
                            Y.x < get_global_size(0) && Y.y < get_global_size(1) && Y.z < get_global_size(2)) {
                            nextSegmentation[LPOS(Y)] = 2; 
                        }
#endif
                      
                        continueGrowing = true;
                    }
                }
            }
        }}}

        if(continueGrowing) {
            // Added new items to list (values of 2)
            stop[0] = 0;
        } else {
            // X was not accepted
#ifdef fast_3d_image_writes
            write_imageui(nextSegmentation, X, 0);
#else
            nextSegmentation[LPOS(X)] = 0;
#endif
        }
    }
}

__kernel void dilate(
        __read_only image3d_t volume, 
#ifdef fast_3d_image_writes
        __write_only image3d_t result
#else
        __global uchar * result
#endif
    ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    if(read_imageui(volume, sampler, pos).x == 1) {
        for(int a = -1; a < 2 ; a++) {
            for(int b = -1; b < 2 ; b++) {
                for(int c = -1; c < 2 ; c++) {
                    int4 nPos = pos + (int4)(a,b,c,0);
#ifdef fast_3d_image_writes
                    write_imageui(result, nPos, 1);
#else
                    // Check if in bounds
                    if(nPos.x >= 0 && nPos.y >= 0 && nPos.z >= 0 &&
                        nPos.x < get_global_size(0) && nPos.y < get_global_size(1) && nPos.z < get_global_size(2))
                    result[LPOS(nPos)] = 1;
#endif
                }
            }
        }
    }
}

__kernel void erode(
        __read_only image3d_t volume, 
#ifdef fast_3d_image_writes
        __write_only image3d_t result
#else
        __global uchar * result
#endif
    ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    int value = read_imageui(volume, sampler, pos).x;
    if(value == 1) {
        bool keep = true;
        for(int a = -1; a < 2 ; a++) {
            for(int b = -1; b < 2 ; b++) {
                for(int c = -1; c < 2 ; c++) {
                    keep = (read_imageui(volume, sampler, pos + (int4)(a,b,c,0)).x == 1 && keep);
                }
            }
        }
#ifdef fast_3d_image_writes
        write_imageui(result, pos, keep ? 1 : 0);
    } else {
        write_imageui(result, pos, 0);
    }
#else
        result[LPOS(pos)] = keep ? 1 : 0;
    } else {
        result[LPOS(pos)] = 0;
    }
#endif
}