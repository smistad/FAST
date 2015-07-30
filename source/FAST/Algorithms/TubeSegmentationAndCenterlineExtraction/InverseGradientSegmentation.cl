__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void initGrowing(
        __read_only image3d_t centerline,
        __write_only image3d_t initSegmentation
        //__read_only image3d_t avgRadius
    ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    if(read_imageui(centerline, sampler, pos).x == 1) {
        float radius = 1;//read_imagef(avgRadius, sampler, pos).x;
        int N = min(max(1, (int)round(radius/2.0f)), 4);

        for(int a = -N; a < N+1; a++) {
        for(int b = -N; b < N+1; b++) {
        for(int c = -N; c < N+1; c++) {
            int4 n;
            n.x = pos.x + a;
            n.y = pos.y + b;
            n.z = pos.z + c;
        if(read_imageui(centerline, sampler, n).x == 0 /*&& length((float3)(a,b,c)) <= N*/)
            write_imageui(initSegmentation, n, 2);
        }}}
    }

}

__kernel void grow(
        __read_only image3d_t currentSegmentation,
        __read_only image3d_t gvf,
        __write_only image3d_t nextSegmentation,
        __global int * stop
    ) {

    int4 X = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    char value = read_imageui(currentSegmentation, sampler, X).x;
    // value of 2, means to check it, 1 means it is already accepted
    if(value == 1) {
        write_imageui(nextSegmentation, X, 1);
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
                        write_imageui(nextSegmentation, X, 1);
                        write_imageui(nextSegmentation, Y, 2);
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
            write_imageui(nextSegmentation, X, 0);
        }
    }
}

// Intialize 3D image to 0
__kernel void init3DImage(
    __write_only image3d_t image
    ) {
    write_imageui(image, (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0), 0);
}

__kernel void dilate(
        __read_only image3d_t volume, 
        __write_only image3d_t result
    ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    if(read_imageui(volume, sampler, pos).x == 1) {
        for(int a = -1; a < 2 ; a++) {
            for(int b = -1; b < 2 ; b++) {
                for(int c = -1; c < 2 ; c++) {
                    int4 nPos = pos + (int4)(a,b,c,0);
                    write_imageui(result, nPos, 1);
                }
            }
        }
    }
}

__kernel void erode(
        __read_only image3d_t volume, 
        __write_only image3d_t result
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
        write_imageui(result, pos, keep ? 1 : 0);
    } else {
        write_imageui(result, pos, 0);
    }
}