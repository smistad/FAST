__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;


//TODO replace all Vector3f as float3? and normal(0) etc with normal.x?
//TODO Function funker ikke? inline everything?
__function float3 getBasePointInPlane(float3 rootPoint, float3 normal, float planeDvalue, int a, int b, int domDir){
    float x, y, z;
    if (domDir == 0){ //domDir: x
        y = a;
        z = b;
        //x
        if (normal.x != 0.0){
            x = -(normal.y*y + normal.z*z + planeDvalue) / normal.x;
        }
    }
    else if (domDir == 1){ //domDir: y
        x = a;
        z = b;
        //y
        if (normal.y != 0.0){
            y = -(normal.x*x + normal.z*z + planeDvalue) / normal.y;
        }
    }
    else if (domDir == 2){ //domDir: z
        x = a;
        y = b;
        //z
        if (normal.z != 0.0){
            z = -(normal.x*x + normal.y*y + planeDvalue) / normal.z;
        }
    }
    return (float3)(x, y, z);
}

__kernel void accumulateFrameToVolume(
    __read_only image2d_t frame,
    __write_only image3d_t volume,
    __global int domDir,
    __global float domVal,
    __global float3 imgNormal,
    __global float3 imgRoot,
    __global float imgPlaneDvalue,
    __global int2 imgSize,
    __global float3 lastNormal,
    __global float3 lastRoot,
    __global float3 nextNormal,
    __global float3 nextRoot,
    __global float16 imgInvTrans,
    __global int2 startOffset
    ){

    //__const float dv, => DV
    //__const float Rmax, => R_MAX
    //__const int3 volSize, => VOL_SIZE_X/Y/Z
    /*
    cl_float16 imgInvTrans = {  
        m00, m01, m02, m03,
        m10, m11, m12, m13,
        m20, m21, m22, m23,
        m30, m31, m32, m33, 
    };
    */
    const int a = get_global_id(0) + startOffset.x;
    const int b = get_global_id(1) + startOffset.y;
    int dataType = get_image_channel_data_type(input);
    //const int4 pos = { get_global_id(0), get_global_id(1), get_global_id(2), 0 };

    //Find basePoint in the plane based on the a and b values
    float3 basePoint = getBasePointInPlane(thisFrameRootPoint, imagePlaneNormal, thisFramePlaneDvalue, a, b, domDir);
    //TODO determine if reasonably close to plane? Elimination/speedup (use inverseTrans)
    //Find distance to last and next frame
    /*
    float d1 = getDistanceAlongNormal(basePoint, imagePlaneNormal, lastFrameRootPoint, lastFrameNormal); //TODO check if correct
    float d2 = getDistanceAlongNormal(basePoint, imagePlaneNormal, nextFrameRootPoint, nextFrameNormal);
    //Calculate half width df and dfDom
    float df = calculateHalfWidth(d1, d2, dv, Rmax);
    float dfDom = df / domVal;

    //Indeks for c-dir range in domDir
    Vector2i cDirRange = getDomDirRange(basePoint, domDir, dfDom, volumeSize);
    //For hver c i c-dir
    for (int c = cDirRange.x; c <= cDirRange.y; c++){
        int3 volumePoint = getVolumePointLocation(a, b, c, domDir);
        if (volumePointOutsideVolume(volumePoint, volumeSize)){
            continue;
        }
        float distance = getPointDistanceAlongNormal(volumePoint, thisFrameRootPoint, imagePlaneNormal);
        float3 intersectionPointWorld = getIntersectionOfPlane(volumePoint, distance, imagePlaneNormal);
        float3 intersectionPointLocal = getLocalIntersectionOfPlane(intersectionPointWorld, thisFrameInverseTransform);
        if (isWithinFrame(intersectionPointLocal, thisFrameSize, 0.5f, 0.5f)){
            float p = getPixelValue(intersectionPointLocal);
            float w = 1 - (distance / df); //Or gaussian for trail
            accumulateValuesInVolume(volumePoint, p, w);
        }
    }
    */
}
