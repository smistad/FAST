__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;


//TODO replace all Vector3f as float3? and normal(0) etc with normal.x?
__function float3 getBasePointInPlane(Vector3f rootPoint, Vector3f normal, float planeDvalue, int a, int b, int domDir){
    float x, y, z;
    if (domDir == 0){ //domDir: x
        y = a;
        z = b;
        //x
        if (normal(0) != 0.0){
            x = -(normal(1)*y + normal(2)*z + planeDvalue) / normal(0);
        }
    }
    else if (domDir == 1){ //domDir: y
        x = a;
        z = b;
        //y
        if (normal(1) != 0.0){
            y = -(normal(0)*x + normal(2)*z + planeDvalue) / normal(1);
        }
    }
    else if (domDir == 2){ //domDir: z
        x = a;
        y = b;
        //z
        if (normal(2) != 0.0){
            z = -(normal(0)*x + normal(1)*y + planeDvalue) / normal(2);
        }
    }
    return Vector3f(x, y, z);
}

__kernel void accumulateFrameToVolume(
    __read_only image2d_t frame,
    __write_only image3d_t volume,
    __const float dv,
    __const float Rmax,
    __const Vector3i volSize,
    __global Vector3f imgNormal,
    __global Vector3f imgRoot,
    __global Vector3f imgPlaneDvalue,
    __global Vector3i imgSize,
    __global Vector3f lastNormal,
    __global Vector3f lastRoot,
    __global Vector3f nextNormal,
    __global Vector3f nextRoot,
    __global AffineTransformation::pointer imgInvTrans,
    __global int domDir,
    __global float domVal,
    ){
    const int a = get_global_id(0);
    const int b = get_global_id(1);
    int dataType = get_image_channel_data_type(input);
    //const int4 pos = { get_global_id(0), get_global_id(1), get_global_id(2), 0 };

    //Find basePoint in the plane based on the a and b values
    Vector3f basePoint = getBasePointInPlane(thisFrameRootPoint, imagePlaneNormal, thisFramePlaneDvalue, a, b, domDir);
    //TODO determine if reasonably close to plane? Elimination/speedup (use inverseTrans)
    //Find distance to last and next frame
    float d1 = getDistanceAlongNormal(basePoint, imagePlaneNormal, lastFrameRootPoint, lastFrameNormal); //TODO check if correct
    float d2 = getDistanceAlongNormal(basePoint, imagePlaneNormal, nextFrameRootPoint, nextFrameNormal);
    //Calculate half width df and dfDom
    float df = calculateHalfWidth(d1, d2, dv, Rmax);
    float dfDom = df / domVal;

    //Indeks for c-dir range in domDir
    Vector2i cDirRange = getDomDirRange(basePoint, domDir, dfDom, volumeSize);
    //For hver c i c-dir
    for (int c = cDirRange(0); c <= cDirRange(1); c++){
        Vector3i volumePoint = getVolumePointLocation(a, b, c, domDir);
        if (volumePointOutsideVolume(volumePoint, volumeSize)){
            continue;
        }
        float distance = getPointDistanceAlongNormal(volumePoint, thisFrameRootPoint, imagePlaneNormal);
        Vector3f intersectionPointWorld = getIntersectionOfPlane(volumePoint, distance, imagePlaneNormal);
        Vector3f intersectionPointLocal = getLocalIntersectionOfPlane(intersectionPointWorld, thisFrameInverseTransform);
        if (isWithinFrame(intersectionPointLocal, thisFrameSize, 0.5f, 0.5f)){
            float p = getPixelValue(intersectionPointLocal);
            float w = 1 - (distance / df); //Or gaussian for trail
            accumulateValuesInVolume(volumePoint, p, w);
        }
    }
    
}

__kernel void normalizeVolume(
    __read_only image3d_t input,
    __write_only image3d_t output
    )
{//Sizes?
    const int4 pos = { get_global_id(0), get_global_id(1), get_global_id(2), 0 }; //x, y, z, component
    int dataType = get_image_channel_data_type(input);

    float p = 0.0f;
    float w = 0.0f;
    if (dataType == CLK_FLOAT) {
        p = read_imagef(input, sampler, pos).x;
        w = read_imagef(input, sampler, pos).y;
    }
    else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        p = read_imageui(input, sampler, pos).x;
        w = read_imageui(input, sampler, pos).y;
    }
    else {
        p = read_imagei(input, sampler, pos).x;
        w = read_imagei(input, sampler, pos).y;
    }
    float finalP = 0.0f;
    if (w > 0.0f && p >= 0.0f){ // w != 0.0 to avoid division error // This other logic to avoid uninitialized voxels
        float finalP = p / w;
    }

    int outputDataType = get_image_channel_data_type(output);
    if (outputDataType == CLK_FLOAT) {
        write_imagef(output, pos, finalP);
    }
    else if (outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(output, pos, round(finalP));
    }
    else {
        write_imagei(output, pos, round(finalP));
    }
}