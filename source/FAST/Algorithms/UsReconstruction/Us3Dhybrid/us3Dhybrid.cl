#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
//#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

float dotProd(float3 A, float3 B){
    float sum = 0.0f;
    sum = A.x * B.x + A.y * B.y + A.z * B.z; //TODO check
    return sum;
}

float3 transformAffine(float3 point, float16 t){
    float3 transformedPoint = { 0.f, 0.f, 0.f };
    float x = point.x;
    float y = point.y;
    float z = point.z;
    transformedPoint.x = t.s0*x + t.s1*y + t.s2*z + t.s3;
    transformedPoint.y = t.s4*x + t.s5*y + t.s6*z + t.s7;
    transformedPoint.z = t.s8*x + t.s9*y + t.sA*z + t.sB;
    //Last line should be 0,0,0,1 anyway; or test and scale?
    return transformedPoint;
}

float getFloat3DirValue(float3 v, int dir){
    if (dir == 0){
        return v.x;
    }
    else if (dir == 1){
        return v.y;
    }
    else {
        return v.z;
    }
}
int getInt3DirValue(int3 v, int dir){
    if (dir == 0){
        return v.x;
    }
    else if (dir == 1){
        return v.y;
    }
    else {
        return v.z;
    }
}
//TODO replace all Vector3f as float3? and normal(0) etc with normal.x?
//TODO Function funker ikke? inline everything?
float3 getBasePointInPlane(float3 rootPoint, float3 normal, float planeDvalue, int a, int b, int domDir){
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

float max3(float a, float b, float c){
    float maximum = a;
    if (b > maximum){
        maximum = b;
    }if (c > maximum){
        maximum = c;
    }
    return maximum;
}

int max2i(int a, int b){
    if (a > b) { return a; }
    else { return b; }
}

int min2i(int a, int b){
    if (a < b) { return a; }
    else { return b; }
}

float max2f(float a, float b){
    if (a > b) { return a; }
    else { return b; }
}

float min2f(float a, float b){
    if (a < b) { return a; }
    else { return b; }
}

float maxCoeff(float3 v){
    float maximum = max3(v.x, v.y, v.z);
    //maximum = max(maximum, v.z);
    return maximum;
}

float getDistanceAlongNormal(float3 point, float3 normal, float3 planePoint, float3 planeNormal){
    //Should handle undefined planePoint and planeNormal TODO check
    if (maxCoeff(planeNormal) < 0.0 || maxCoeff(planePoint) < 0.0){ //TODO change?
        return 0.0f;
    }
    //P0 = planePoint
    //L0 = point in world
    //N = planeNormal
    //L = normal from point/origin
    float divisor = dotProd(normal, planeNormal); // normal.dot(planeNormal);
    float dividend = dotProd((planePoint - point), planeNormal); //(planePoint - point).dot(planeNormal);
    if (divisor == 0.0){
        if (dividend == 0.0){
            //Parallell with the plane, IN the plane
            return 0.0f;
        }
        else {
            //Parallell with the plane, but NOT in it
            return 1000.0f; //OR Infinite?
        }
    }
    else {
        float distance = dividend / divisor;
        return fabs(distance); //TODO make fabs()?
    }
}

float calculateHalfWidth(float d1, float d2){//, float dv, float Rmax){
    //float furthestNeighbour = max(d1, d2);
    float maxTotal = max3(d1, d2, DV); //max(furthestNeighbour, DV);
    float results = min2f(maxTotal, R_MAX); //min
    return results;
}

int2 getDomDirRange(float3 basePoint, int domDir, float dfDom){//, int3 volumeSize){
    //getFloatDirValue(float3 v, int dir)
    float rootC = getFloat3DirValue(basePoint, domDir);//basePoint(domDir);
    int domDirSize; {
        if (domDir == 0) { domDirSize = VOL_SIZE_X; }
        else if (domDir == 1) { domDirSize = VOL_SIZE_Y; }
        else { domDirSize = VOL_SIZE_Z; }
    }
    //int domDirSize = getInt3DirValue(volumeSize, domDir); //volumeSize(domDir);
    int startC = max2i(0, ceil(rootC - dfDom)); //max
    int endC = min2i(floor(rootC + dfDom), domDirSize - 1);// min2i(ceil(rootC + dfDom), domDirSize - 1);
    int2 results = { startC, endC };
    return results;
}

int3 getVolumePointLocation(int a, int b, int c, int domDir){
    //int x, y, z;
    int3 result;
    if (domDir == 0){ //domDir: x
        result.x = c;
        result.y = a;
        result.z = b;
    }
    else if (domDir == 1){ //domDir: y
        result.x = a;
        result.y = c;
        result.z = b;
    }
    else if (domDir == 2){ //domDir: z
        result.x = a;
        result.y = b;
        result.z = c;
    }
    return result; //Vector3i(x, y, z);
}

bool volumePointOutsideVolume(int3 volumePoint){//, Vector3i volumeSize){
    if (volumePoint.x < 0 || volumePoint.x >= VOL_SIZE_X){
        return true;
    }
    if (volumePoint.y < 0 || volumePoint.y >= VOL_SIZE_Y){
        return true;
    }
    if (volumePoint.z < 0 || volumePoint.z >= VOL_SIZE_Z){
        return true;
    }
    /*
    for (int k = 0; k < 3; k++){
        int point = volumePoint(k);
        float size = volumeSize(k);
        if (point < 0 || point >= size){
            return true;
        }
    }*/
    return false;
}

float getPointDistanceAlongNormal(int3 A, float3 B, float3 normal){
    // |(B-A).dot(normal)|
    float3 Af = {(float)A.x, (float)A.y, (float)A.z};
    float3 diff = (B - Af);
    //float3 diff = float3()
    float distance = dotProd(diff, normal);// diff.dot(normal);
    //float distance = fabs(prod); //fabs((B - A).dot(normal));
    return distance; // IKKE abs-val for now... Needs it further. distance;
}

float3 getIntersectionOfPlane(int3 startPoint, float distance, float3 normalVector){
    float3 startPointF = {(float)startPoint.x, (float)startPoint.y, (float)startPoint.z};//float3(startPoint(0), startPoint(1), startPoint(2));
    float3 moveDist = { distance*normalVector.x, distance*normalVector.y, distance*normalVector.z }; //float3(normalVector*distance);
    //float3 moveDist = float3(normalVector(0)*distance, normalVector(1)*distance, normalVector(2)*distance);
    float3 endPoint = startPointF + moveDist;
    //TODO add check towards target plane ?? "Project" into target plane?
    return endPoint;
}

float3 getLocalIntersectionOfPlane(float3 intersectionPointWorld, float16 imgInvTrans){ 
    //AffineTransformation::pointer frameInverseTransform){
    //Plocal = InverseTransform * Pglobal
    //TODO
    float3 intersectionPointLocal = transformAffine(intersectionPointWorld, imgInvTrans);
    //float3 intersectionPointLocal = frameInverseTransform->multiply(intersectionPointWorld);
    return intersectionPointLocal;
}
//getInt3DirValue
bool isWithinFrame(float3 intersectionPointLocal, int2 frameSize){//, float bufferXY, float bufferZ){
    //Ev use untransformed boundingBox?
    if (-BUFFER_Z > intersectionPointLocal.z > BUFFER_Z){ //If z too out of bounds //should not occure? Transformation error?
        //float badZ = intersectionPointLocal.z;
        return false;
    }
    bool inside = true;
    for (int axis = 0; axis <= 1; axis++){
        //For each axis X and Y
        float point = getFloat3DirValue(intersectionPointLocal, axis);// intersectionPointLocal(axis);
        //uint size = getInt3DirValue(frameSize, axis);// frameSize(axis);
        int size = frameSize.x;
        if (axis == 1) size = frameSize.y;
        if (point + BUFFER_XY < 0.0f){ //Bigger val than frame on this axis
            inside = false;
            break;
        }
        if (point - BUFFER_XY > (float)(size - 1)){ //Bigger val than frame on this axis
            inside = false;
            break;
        }
    }
    return inside;
}

float getFrameValue(image2d_t frame, int2 pos, int dataType){
    //int2 realPos = pos.xy;
    float p;
    if (dataType == CLK_FLOAT) {
        p = read_imagef(frame, sampler, pos).x;
    }
    else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        p = read_imageui(frame, sampler, pos).x;
    }
    else {
        p = read_imagei(frame, sampler, pos).x;
    }
    return p;
}

float getPixelValueData(unsigned char* frameData, float3 point, int2 frameSize, int dataType){ //Vector3f point){
    //Gets frameAccess from Us3Dhybrid class
    float x = point.x;// (0);
    float y = point.y;// (1);
    int z = round(point.z);// (2));
    int xCeil = ceil(x);
    int yCeil = ceil(y);
    if (xCeil < 0 || yCeil < 0 || z != 0 || xCeil > frameSize.x - 1 || yCeil > frameSize.y - 1){
        //Throw error? Should not need to occur if appropriate bufferXY in last function
        return 0.0f;
    }
    int sizeX = frameSize.x;
    int xFloor = floor(x);
    int yFloor = floor(y);
    int locMinMin = (xFloor + yFloor * sizeX) * 3;
    int locMinMax = (xFloor + yCeil * sizeX) * 3;
    int locMaxMin = (xCeil + yFloor * sizeX) * 3;
    int locMaxMax = (xCeil + yCeil * sizeX) * 3;
    //int3 locMinMin = (int3)(xFloor, yFloor, z);
    //int3 locMinMax = (int3)(xFloor, yCeil, z);
    //int3 locMaxMin = (int3)(xCeil, yFloor, z);
    //int3 locMaxMax = (int3)(xCeil, yCeil, z);
    if (xFloor < 0){
        //int loc = (xCeil + yCeil*frameSize.x() + z*frameSize.x()*frameSize.y())*firstFrame->getNrOfComponents() + 0;// channel;
        //int locMaxMax = (xCeil + yCeil * sizeX) * 3;
        unsigned char pMaxMax = frameData[locMaxMax];
        //float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        if (yFloor < 0){
            //Case 1
            return (float)pMaxMax;
        }
        else {
            //Case 3
            //int locMaxMin = (xCeil + yFloor * sizeX) * 3;
            unsigned char pMaxMin = frameData[locMaxMin];
            //float pMaxMin = frameAccess->getScalar(Vector3i(xCeil, yFloor, z));
            float v = y - yFloor;
            float pRight = ((float)pMaxMin)*(1.0f - v) + ((float)pMaxMax)*v;
            return pRight;
        }
    }
    else if (yFloor < 0){
        //Case 2
        float u = x - xFloor;
        //int locMinMax = (xFloor + yCeil * sizeX) * 3;
        //int locMaxMax = (xCeil + yCeil * sizeX) * 3;
        unsigned char pMinMax = frameData[locMinMax];
        unsigned char pMaxMax = frameData[locMaxMax];
        //float pMinMax = frameAccess->getScalar(Vector3i(xFloor, yCeil, z));
        //float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        float pBot = ((float)pMinMax)*(1.0f - u) + ((float)pMaxMax)*u;
        return pBot;
    }
    else {
        //int loc = (xFloor + yFloor*frameSize.x() + z*frameSize.x()*frameSize.y())*firstFrame->getNrOfComponents() + 0;// channel;
        /*int locMinMin = (xFloor + yFloor * sizeX) * 3;
        int locMinMax = (xFloor + yCeil * sizeX) * 3;
        int locMaxMin = (xCeil + yFloor * sizeX) * 3;
        int locMaxMax = (xCeil + yCeil * sizeX) * 3;*/
        unsigned char pMinMin = frameData[locMinMin];
        unsigned char pMinMax = frameData[locMinMax];
        unsigned char pMaxMin = frameData[locMaxMin];
        unsigned char pMaxMax = frameData[locMaxMax];
        /*
        float pMinMin = frameAccess->getScalar(Vector3i(xFloor, yFloor, z)); //ev 0 for point(2) or round?
        float pMinMax = frameAccess->getScalar(Vector3i(xFloor, yCeil, z));
        float pMaxMin = frameAccess->getScalar(Vector3i(xCeil, yFloor, z));
        float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        */
        //Calculate horizontal interpolation
        float u = x - xFloor; //float u = point(0) - floor(point(0));
        float pTop = ((float)pMinMin)*(1.0f - u) + ((float)pMaxMin)*u;
        float pBot = ((float)pMinMax)*(1.0f - u) + ((float)pMaxMax)*u;
        //Calculate final vertical interpolation
        float v = y - yFloor; //float v = point(1) - floor(point(1));
        float pValue = pTop*(1.0f - v) + pBot*v;//pBot*(1 - v) + pTop*v;
        return pValue;
    }
}

float getPixelValue(image2d_t frame, float3 point, int2 frameSize, int dataType){
    //Gets frameAccess from Us3Dhybrid class
    float x = point.x;// (0);
    float y = point.y;// (1);
    int z = round(point.z);// (2));
    int xCeil = ceil(x);
    int yCeil = ceil(y);
    if (xCeil < 0 || yCeil < 0 || z != 0 || xCeil > frameSize.x - 1 || yCeil > frameSize.y - 1){
        //Throw error? Should not need to occur if appropriate bufferXY in last function
        return 0.0f;
    }
    int sizeX = frameSize.x;
    int xFloor = floor(x);
    int yFloor = floor(y);
    //int locMinMin = (xFloor + yFloor * sizeX) * 3;
    //int locMinMax = (xFloor + yCeil * sizeX) * 3;
    //int locMaxMin = (xCeil + yFloor * sizeX) * 3;
    //int locMaxMax = (xCeil + yCeil * sizeX) * 3;
    int2 locMinMin = (int2)(xFloor, yFloor);
    int2 locMinMax = (int2)(xFloor, yCeil);
    int2 locMaxMin = (int2)(xCeil, yFloor);
    int2 locMaxMax = (int2)(xCeil, yCeil);
    //pos.xyz = ( x, y, z );
    if (xFloor < 0){
        float pMaxMax = getFrameValue(frame, locMaxMax, dataType);
        //float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        if (yFloor < 0){
            //Case 1
            return pMaxMax;
        }
        else {
            //Case 3
            float pMaxMin = getFrameValue(frame, locMaxMin, dataType);
            //float pMaxMin = frameAccess->getScalar(Vector3i(xCeil, yFloor, z));
            float v = y - yFloor;
            float pRight = pMaxMin*(1.0f - v) + pMaxMax*v;
            return pRight;
        }
    }
    else if (yFloor < 0){
        //Case 2
        float u = x - xFloor;
        float pMinMax = getFrameValue(frame, locMinMax, dataType);
        float pMaxMax = getFrameValue(frame, locMaxMax, dataType);
        //float pMinMax = frameAccess->getScalar(Vector3i(xFloor, yCeil, z));
        //float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        float pBot = pMinMax*(1.0f - u) + pMaxMax*u;
        return pBot;
    }
    else {
        float pMinMin = getFrameValue(frame, locMinMin, dataType);
        float pMinMax = getFrameValue(frame, locMinMax, dataType);
        float pMaxMin = getFrameValue(frame, locMaxMin, dataType);
        float pMaxMax = getFrameValue(frame, locMaxMax, dataType);
        //float pMinMin = frameAccess->getScalar(Vector3i(xFloor, yFloor, z)); //ev 0 for point(2) or round?
        //float pMinMax = frameAccess->getScalar(Vector3i(xFloor, yCeil, z));
        //float pMaxMin = frameAccess->getScalar(Vector3i(xCeil, yFloor, z));
        //float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        
        //Calculate horizontal interpolation
        float u = x - xFloor; //float u = point.x - floor(point.x);
        float pTop = pMinMin*(1.0f - u) + pMaxMin*u;
        float pBot = pMinMax*(1.0f - u) + pMaxMax*u;
        //Calculate final vertical interpolation
        float v = y - yFloor; //float v = point.y - floor(point.y);
        float pValue = pTop*(1.0f - v) + pBot*v;//pBot*(1 - v) + pTop*v;
        return pValue;
    }
}
/*
float get3DvolumeValue(image3d_t volume, int3 pos, int component, int outputDataType){
    float p = 0.0f;
    int4 realPos = (int4)( pos.x, pos.y, pos.z, 0 );
    //OR TODO access both as int2 and return this // more efficient?
    float4 res;
    if (outputDataType == CLK_FLOAT) {
        res = read_imagef(volume, sampler, realPos);
        //if (component == 0) p = read_imagef(volume, sampler, realPos).x;
        //else p = read_imagef(volume, sampler, realPos).y;
    }
    *
    else if (outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        if (component == 0) p = read_imageui(volume, sampler, realPos).x;
        else p = read_imageui(volume, sampler, realPos).y;
    }
    else {
        if (component == 0) p = read_imagei(volume, sampler, realPos).x;
        else p = read_imagei(volume, sampler, realPos).y;
    }
    *
    return p;
}

void set3DvolumeValue(image3d_t volume, int3 pos, float value, int component, int outputDataType){
    //int outputDataType = get_image_channel_data_type(volume);
    int4 realPos = { pos.x, pos.y, pos.z, component };
    if (outputDataType == CLK_FLOAT) {
        write_imagef(volume, realPos, value);// .x;
        //if (component == 0)
        //else write_imagef(volume, pos, value).y;
        //write_imagef(output, pos, finalP);
    }
    else if (outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(volume, realPos, round(value));// .x;
        //if (component == 0)
        //else write_imageui(volume, realPos, round(value));// .y;
        //write_imageui(output, pos, round(finalP));
    }
    else {
        write_imagei(volume, realPos, round(value));// .x;
        //if (component == 0)
        //else write_imagei(volume, realPos, round(value));// .y;
        //write_imagei(output, pos, round(finalP));
    }
}
*/ 

float get3DvolumeVectorValue(__global float * volume, int3 pos, int component){
    //assumes 2 components
    float p = 0.0f;
    int loc = component + pos.x*VOL_SIZE_X * 2 + pos.y*VOL_SIZE_X*VOL_SIZE_Y * 2 + pos.z*VOL_SIZE_X*VOL_SIZE_Y*VOL_SIZE_Z * 2;
    p = volume[loc];
    return p;
}

void set3DvolumeVectorValue(__global float * volume, int3 pos, float value, int component){
    int loc = component + pos.x*VOL_SIZE_X * 2 + pos.y*VOL_SIZE_X*VOL_SIZE_Y * 2 + pos.z*VOL_SIZE_X*VOL_SIZE_Y*VOL_SIZE_Z * 2;
    volume[loc] = value;
}

void GetSemaphor(__global int * semaphor) {
    int occupied = atom_xchg(semaphor, 1);
    while (occupied > 0)
    {
        occupied = atom_xchg(semaphor, 1);
    }
}

void ReleaseSemaphor(__global int * semaphor)
{
    int prevVal = atom_xchg(semaphor, 0);
}

void accumulateValuesInVolumeData(__global float * volume, int3 volumePoint, float p, float w, int outputDataType, __global int * semaphor){
    //volAccess available from Us3Dhybrid as ImageAccess::pointer
    //barrier(CLK_GLOBAL_MEM_FENCE);//TODO add a lock to these????
    int loc = volumePoint.x + volumePoint.y*VOL_SIZE_X + volumePoint.z*VOL_SIZE_XtY;//VOL_SIZE_X*VOL_SIZE_Y;
    GetSemaphor(&semaphor[loc]);
    {
        //component + pos.x*VOL_SIZE_X * 2 + pos.y*VOL_SIZE_X*VOL_SIZE_Y * 2 + pos.z*VOL_SIZE_X*VOL_SIZE_Y*VOL_SIZE_Z * 2
        int loc2 = loc * 2;
        float oldP = volume[loc2];//get3DvolumeVectorValue(volume, volumePoint, 0);
        //0.0f;// get3DvolumeValue(volume, volumePoint, 0, outputDataType);
        float oldW = volume[loc2 + 1];//get3DvolumeVectorValue(volume, volumePoint, 1);
        //0.0f;//get3DvolumeValue(volume, volumePoint, 1, outputDataType);
        //float oldP = volAccess->getScalar(volumePoint, 0); //out of bounds????
        //float oldW = volAccess->getScalar(volumePoint, 1);
        if (oldP < 0.0f) oldP = 0.0f;
        if (oldW < 0.0f) oldW = 0.0f;
        float newP = oldP + p*w;
        float newW = oldW + w;
        volume[loc2] = newP;
        volume[loc2 + 1] = newW;  //ev bruk int og atomic increment (atom_inc)
        //or atomic_add (for int/long/uint/ulong) to add addition
        //set3DvolumeVectorValue(volume, volumePoint, newP, 0);// , outputDataType);
        //set3DvolumeVectorValue(volume, volumePoint, newW, 1);// , outputDataType);
        /**/
        //volAccess->setScalar(volumePoint, newP, 0);
        //volAccess->setScalar(volumePoint, newW, 1);
    }
    ReleaseSemaphor(&semaphor[loc]);
}

void accumulateValuesInVolumeUInt(__global unsigned int * volume, int3 volumePoint, float p, float w){
    int locP = (volumePoint.x + volumePoint.y*VOL_SIZE_X + volumePoint.z*VOL_SIZE_XtY) * 2;//VOL_SIZE_X*VOL_SIZE_Y;
    int locW = locP + 1;
    unsigned int addW = round((w * UINT_GRANULARITY)); //resolution on 100
    unsigned int addP = round((p * w * UINT_GRANULARITY)); // ev (p*w)/100?
    atomic_add(&volume[locP], addP);
    atomic_add(&volume[locW], addW);
}

__kernel void accumulateFrameToVolume(
    __read_only image2d_t frame,
    __global unsigned int* volume,
    __private const int domDir,
    __private const float domVal,
    __private const float3 imgNormal,
    __private const float3 imgRoot,
    __private const float imgPlaneDvalue,
    __private const int2 imgSize,
    __private const float3 lastNormal,
    __private const float3 lastRoot,
    __private const float3 nextNormal,
    __private const float3 nextRoot,
    __private const float16 imgInvTrans
    ){
    //__read_write image3d_t volume, => RESERVED for OpenCL2+
    //__const float dv, => DV
    //__const float Rmax, => R_MAX
    //__const int3 volSize, => VOL_SIZE_X/Y/Z
    //__global float* volume, => Replaced by unsigned int
    //__global unsigned int* volume,
    //__private const int outputDataType,
    
    /*
    cl_float16 imgInvTrans = {  
        m00, m01, m02, m03,
        m10, m11, m12, m13,
        m20, m21, m22, m23,
        m30, m31, m32, m33, 
    };
    */
    const int a = get_global_id(0);// +startOffset.x;
    const int b = get_global_id(1);// +startOffset.y;
    int dataType = get_image_channel_data_type(frame);
    //int outputDataType = get_image_channel_data_type(volume);
    //const int4 pos = { get_global_id(0), get_global_id(1), get_global_id(2), 0 };

    //Find basePoint in the plane based on the a and b values
    float3 basePoint = getBasePointInPlane(imgRoot, imgNormal, imgPlaneDvalue, a, b, domDir);
    //TODO determine if reasonably close to plane? Elimination/speedup (use inverseTrans)
    //Find distance to last and next frame
    /*int3 volumePointZero = (int3)(basePoint.x, basePoint.y, basePoint.z);
    float p = 5.0f;// 256.0f;
    float3 crossLocal = getLocalIntersectionOfPlane(basePoint, imgInvTrans);
    if (isWithinFrame(crossLocal, imgSize)){
        p = getPixelValue(frame, crossLocal, imgSize, dataType);
    }
    float w = 1.0f;*/
    //ran[0] = 1;
    //atom_inc(&ran[0]);
    //accumulateValuesInVolumeData(volume, volumePointZero, p, w, outputDataType, semaphor);
    //accumulateValuesInVolumeUInt(volume, volumePointZero, p, w);
    //return;
    float d1 = getDistanceAlongNormal(basePoint, imgNormal, lastRoot, lastNormal); //TODO check if correct
    float d2 = getDistanceAlongNormal(basePoint, imgNormal, nextRoot, nextNormal);
    //Calculate half width df and dfDom
    float df = calculateHalfWidth(d1, d2);// , dv, Rmax);
    float dfDom = df / domVal;

    float gaussianK = 1.0f / (df*SQRT_2_PI);
    
    //Indeks for c-dir range in domDir
    int2 cDirRange = getDomDirRange(basePoint, domDir, dfDom);// , volumeSize);
    //For hver c i c-dir
    for (int c = cDirRange.x; c <= cDirRange.y; c++){
        int3 volumePoint = getVolumePointLocation(a, b, c, domDir);
        if (volumePointOutsideVolume(volumePoint)){ //, volumeSize)){
            continue;
        }
        float distance = getPointDistanceAlongNormal(volumePoint, basePoint, imgNormal);
        float absDistance = fabs(distance);
        if (absDistance > df){
            continue;
        }
        float3 intersectionPointWorld = getIntersectionOfPlane(volumePoint, distance, imgNormal);
        float3 intersectionPointLocal = getLocalIntersectionOfPlane(intersectionPointWorld, imgInvTrans);
        if (isWithinFrame(intersectionPointLocal, imgSize)){//, 0.5f, 0.5f)){
            //float p = 256.0f;// getPixelValue(frame, intersectionPointLocal, imgSize, dataType); //TODO FIX
            //float w = 1.0f;// 1 - (fabs(distance) / df); //Or gaussian for trail
            float p = getPixelValue(frame, intersectionPointLocal, imgSize, dataType); //TODO FIX
            //float w = 1.0f - (absDistance / df); //Or gaussian for trail
            float w = gaussianK * exp2((-0.5f*absDistance*absDistance) / (df*df));
            //float w = 1.0f - ((absDistance*absDistance) / (df*df));
            //float w = sqrt((absDistance / df));
            //accumulateValuesInVolumeData(volume, volumePoint, p, w, outputDataType, semaphor); //TODO FIX
            accumulateValuesInVolumeUInt(volume, volumePoint, p, w);
        }
        
    }
    
}

__kernel void addPNNFrame(
    __read_only image2d_t frame,
    __global unsigned int* volume,
    __private const int domDir,
    __private const float domVal,
    __private const float3 imgNormal,
    __private const float3 imgRoot,
    __private const float imgPlaneDvalue,
    __private const int2 imgSize,
    __private const float16 imgTrans
    ){

    const int x = get_global_id(0);
    const int y = get_global_id(1);
    int dataType = get_image_channel_data_type(frame);

    float3 pos = (float3)(x, y, 0);
    
    float3 volPos = transformAffine(pos, imgTrans);
    int3 volPosDiscrete = (int3)(volPos.x, volPos.y, volPos.z);
    if (volumePointOutsideVolume(volPosDiscrete)){
        return;
    }
    int2 pos2 = (int2)(x, y);
    float frameP = getFrameValue(frame, pos2, dataType);
    //Ev use normal to add lower weight value to next/previous
    accumulateValuesInVolumeUInt(volume, volPosDiscrete, frameP, 1.0f);
    //volAccess->setScalar(worldPosDiscrete, frameP, 0);
    //volAccess->setScalar(worldPosDiscrete, 1.0f, 1);
    //float getFrameValue(image2d_t frame, int2 pos, int dataType)
    //float3 transformAffine(float3 point, float16 t)
    /*
    Vector3f pos = Vector3f(x, y, 0);
    //AffineTransformation::pointer imgTransform = SceneGraph::getAffineTransformationFromData(frame);
    AffineTransformation::pointer imgTransform = getTransform(frame);
    Vector3f worldPos = imgTransform->multiply(pos);
    Vector3i worldPosDiscrete = Vector3i(round(worldPos(0)), round(worldPos(1)), round(worldPos(2)));
    if (volumePointOutsideVolume(worldPosDiscrete, volumeSize)){
        continue;
    }
    float frameP = frameAccess->getScalar(Vector3i(x, y, 0));
    //int loc = x*width + y*width*height;
    //Vector3ui sizes = frame->getSize();
    //uint loc = x*nrOfComponents + y*width*nrOfComponents;
    //x*nrOfComponents + y*nrOfComponents*width + z*nrOfComponents*width*height
    // uint address = (position.x() + position.y()*size.x() + position.z()*size.x()*size.y())*image->getNrOfComponents() + channel;
    //float frameP2 = frameValues[loc];
    volAccess->setScalar(worldPosDiscrete, frameP, 0);
    volAccess->setScalar(worldPosDiscrete, 1.0f, 1);
    */
}

float  readAndNormalize(__global unsigned int* volume, int loc, float defValue){
    int locReadP = loc * 2;
    int locReadW = loc * 2 + 1;
    unsigned int p = volume[locReadP];
    unsigned int w = volume[locReadW];
    float value = defValue;
    if (w != 0)
        value = (float)p / (float)w;
    return value;
}

void setLocalValue(__local float* localStorage, int x, int y, int z, float valA){
    int locA = (x + y*LSIZE_MEM_X + z*LSIZE_MEM_XtY);
    localStorage[locA] = valA; //?
}

__kernel void normalizeHoleFillVolume(
    __write_only image3d_t outputVolume,
    __global unsigned int * volume
    ){
    //,__local float * localStorage
    //Depends upon defined
    // VOL_SIZE_X/Y/Z/XtY
    // LSIZE_MEM_X/Y/Z/XtY = LocalMemory Size
    // LSIZE_X/Y/Z = workSize in local by direction
    const int xG = get_global_id(0);
    const int yG = get_global_id(1);
    const int zG = get_global_id(2);
    //int3 pos = (int3)(xG, yG, zG);
    int4 pos = (int4)(xG, yG, zG, 0);

    //const int x = get_local_id(0);
   // const int y = get_local_id(1);
   // const int z = get_local_id(2);

    //LOCAL OFFSET?
    int locGlobal = (xG + yG*VOL_SIZE_X + zG*VOL_SIZE_XtY); //Component later
    //float voxelValue = 0.0f; // readAndNormalize(volume, locGlobal, 0.0f);
    float voxelValue = readAndNormalize(volume, locGlobal, -1.0f);
    /*if (false){
        //READ A and normalize
        int AaddX = 0; int AaddY = 0; int AaddZ = 0;
        {
            int locGlobalA = ((xG + AaddX) + (yG + AaddY)*VOL_SIZE_X + (zG + AaddZ)*VOL_SIZE_XtY); //Component later
            float valA = readAndNormalize(volume, locGlobalA, -1.0f);
            setLocalValue(localStorage, (x + AaddX), (y + AaddY), (z + AaddZ), valA);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        //READ B and normalize
        int BaddX = 0; int BaddY = LSIZE_Y; int BaddZ = 0;
        if ((y + BaddY) < LSIZE_MEM_Y && (yG + BaddY) < VOL_SIZE_Y){
            int locGlobalB = ((xG + BaddX) + (yG + BaddY)*VOL_SIZE_X + (zG + BaddZ)*VOL_SIZE_XtY); //Component later
            float valB = readAndNormalize(volume, locGlobalB, -1.0f);
            setLocalValue(localStorage, (x + BaddX), (y + BaddY), (z + BaddZ), valB);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        //READ C and normalize
        int CaddX = LSIZE_X; int CaddY = 0; int CaddZ = 0;
        if ((x + CaddX) < LSIZE_MEM_X && (xG + CaddX) < VOL_SIZE_X){
            int locGlobalC = ((xG + CaddX) + (yG + CaddY)*VOL_SIZE_X + (zG + CaddZ)*VOL_SIZE_XtY); //Component later
            float valC = readAndNormalize(volume, locGlobalC, -1.0f);
            setLocalValue(localStorage, (x + CaddX), (y + CaddY), (z + CaddZ), valC);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        //READ D and normalize
        int DaddX = LSIZE_X; int DaddY = LSIZE_Y; int DaddZ = 0;
        if ((x + DaddX) < LSIZE_MEM_X && (xG + DaddX) < VOL_SIZE_X
            && (y + DaddY) < LSIZE_MEM_Y && (yG + DaddY) < VOL_SIZE_Y){
            int locGlobalD = ((xG + DaddX) + (yG + DaddY)*VOL_SIZE_X + (zG + DaddZ)*VOL_SIZE_XtY); //Component later
            float valD = readAndNormalize(volume, locGlobalD, -1.0f);
            setLocalValue(localStorage, (x + DaddX), (y + DaddY), (z + DaddZ), valD);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        int addZ = LSIZE_Z;
        if ((z + addZ) < LSIZE_MEM_Z && (zG + addZ) < VOL_SIZE_Z){
            //READ E and normalize
            int EaddX = 0; int EaddY = 0;
            {
                int locGlobalE = ((xG + EaddX) + (yG + EaddY)*VOL_SIZE_X + (zG + addZ)*VOL_SIZE_XtY); //Component later
                float valE = readAndNormalize(volume, locGlobalE, -1.0f);
                setLocalValue(localStorage, (x + EaddX), (y + EaddY), (z + addZ), valE);
            }
            //MEMBLOCK?
            //READ G and normalize
            int GaddX = 0; int GaddY = LSIZE_Y;
            if ((y + GaddY) < LSIZE_MEM_Y && (yG + GaddY) < VOL_SIZE_Y){
                int locGlobalG = ((xG + GaddX) + (yG + GaddY)*VOL_SIZE_X + (zG + addZ)*VOL_SIZE_XtY); //Component later
                float valG = readAndNormalize(volume, locGlobalG, -1.0f);
                setLocalValue(localStorage, (x + GaddX), (y + GaddY), (z + addZ), valG);
            }
            //MEMBLOCK?
            //READ F and normalize
            int FaddX = LSIZE_X; int FaddY = 0;
            if ((x + FaddX) < LSIZE_MEM_X && (xG + FaddX) < VOL_SIZE_X){
                int locGlobalF = ((xG + FaddX) + (yG + FaddY)*VOL_SIZE_X + (zG + addZ)*VOL_SIZE_XtY); //Component later
                float valF = readAndNormalize(volume, locGlobalF, -1.0f);
                setLocalValue(localStorage, (x + FaddX), (y + FaddY), (z + addZ), valF);
            }
            //MEMBLOCK?
            //READ H and normalize
            int HaddX = LSIZE_X; int HaddY = LSIZE_Y;
            if ((x + HaddX) < LSIZE_MEM_X && (xG + HaddX) < VOL_SIZE_X
                && (y + HaddY) < LSIZE_MEM_Y && (yG + HaddY) < VOL_SIZE_Y){
                int locGlobalH = ((xG + HaddX) + (yG + HaddY)*VOL_SIZE_X + (zG + addZ)*VOL_SIZE_XtY); //Component later
                float valH = readAndNormalize(volume, locGlobalH, -1.0f);
                setLocalValue(localStorage, (x + HaddX), (y + HaddY), (z + addZ), valH);
            }
        }
    }*/
    if (voxelValue < 0.0f){//true){
        //MEMBLOCK //REALLY IMPORTANT ONE
        barrier(CLK_LOCAL_MEM_FENCE);
        //All data is read to local, perform calculation
        

        int minX = xG; //x; //or max this and 0? or sampler handles it?
        int minY = yG; //y;
        int minZ = zG; //z;
        int maxX = minX + HALF_WIDTH_X2;
        int maxY = minY + HALF_WIDTH_X2;
        int maxZ = minZ + HALF_WIDTH_X2;

        float accumulationValue = 0.0f;
        int counter = 0;
        for (int xi = minX; xi <= maxX; xi++){
            for (int yi = minY; yi <= maxY; yi++){
                for (int zi = minZ; zi <= maxZ; zi++){
                    int loc = (xi + yi*LSIZE_MEM_X + zi*LSIZE_MEM_XtY);
                    //float locValue = localStorage[loc];
                    float locValue = readAndNormalize(volume, loc, -1.0f);
                    if (locValue >= 0.0f){ //ev > -0.5? for inaccuracy?
                        accumulationValue += locValue;
                        counter++;
                    }
                }
            }
        }
        //float 
        voxelValue = accumulationValue / counter;
    }
    
    //barrier(CLK_LOCAL_MEM_FENCE);
    int outputDataType = get_image_channel_data_type(outputVolume);
    if (outputDataType == CLK_FLOAT) {
        write_imagef(outputVolume, pos, voxelValue);
    }
    else if (outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(outputVolume, pos, round(voxelValue));
    }
    else {
        write_imagei(outputVolume, pos, round(voxelValue));
    }
}

__kernel void inc(global int * num){
    atom_inc(&num[0]);
}

/*
float getFrameValue(image2d_t frame, int2 pos, int dataType){
    //int2 realPos = pos.xy;
    float p;
    if (dataType == CLK_FLOAT) {
        p = read_imagef(frame, sampler, pos).x;
    }
    else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        p = read_imageui(frame, sampler, pos).x;
    }
    else {
        p = read_imagei(frame, sampler, pos).x;
    }
    return p;
}
*/

__kernel void normalizeVolume(
    __write_only image3d_t outputVolume,
    __global unsigned int* volume
    ){
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    int4 pos = (int4)(x, y, z, 0);
    int loc = (x + y*VOL_SIZE_X + z*VOL_SIZE_XtY);
    float value = readAndNormalize(volume, loc, 0.0f);
    /*
    int locP = (x + y*VOL_SIZE_X + z*VOL_SIZE_XtY) * 2;//VOL_SIZE_X*VOL_SIZE_Y;
    int locW = locP + 1;
    
    //float value = ((float)volume[locP]) / ((float)volume[locW]);
    unsigned int p = volume[locP];
    unsigned int w = volume[locW];
    unsigned int div = 128;
    float value = 0.0f;// 128.0f;
    if (w != 0){
        value = (float)p / (float) w;
        div = p / w;
        //value = div; // * 65535.0f;
        //value = (float)div;// *65535.0f;
    }
    */
    //writeVolumeValue();
    int outputDataType = get_image_channel_data_type(outputVolume);
    if (outputDataType == CLK_FLOAT) {
        write_imagef(outputVolume, pos, value);
    }
    else if (outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(outputVolume, pos, round(value)); //div
    }
    else {
        write_imagei(outputVolume, pos, round(value));
    }
}