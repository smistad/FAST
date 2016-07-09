__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

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

int max2i(int a, int b){
    if (a > b) { return a; }
    else { return b; }
}

int min2i(int a, int b){
    if (a < b) { return a; }
    else { return b; }
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
    return false;
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

void accumulateValuesInVolumeUInt(__global unsigned int * volume, int3 volumePoint, float p, float w){
    int locP = (volumePoint.x + volumePoint.y*VOL_SIZE_X + volumePoint.z*VOL_SIZE_XtY) * 2;//VOL_SIZE_X*VOL_SIZE_Y;
    int locW = locP + 1;
    unsigned int addW = round((w * UINT_GRANULARITY)); //resolution on 100
    unsigned int addP = round((p * w * UINT_GRANULARITY)); // ev (p*w)/100?
    atomic_add(&volume[locP], addP);
    atomic_add(&volume[locW], addW);
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


void setLocalValue(__local float * sharedMem, int x, int y, int z, float valA){
    int locA = (x + y*LSIZE_MEM_X + z*LSIZE_MEM_XtY);
    sharedMem[locA] = valA; //?
}



__kernel void normalizeHoleFillVolume(
    __write_only image3d_t outputVolume,
    __global unsigned int * volume, 
    __local float * sharedMem
    ){
    //,__local float * sharedMem
    //,__local float * sharedMem
    //Depends upon defined
    // VOL_SIZE_X/Y/Z/XtY
    // LSIZE_MEM_X/Y/Z/XtY = LocalMemory Size
    // LSIZE_X/Y/Z = workSize in local by direction
    const int xG = get_global_id(0);
    const int yG = get_global_id(1);
    const int zG = get_global_id(2);
    //int3 pos = (int3)(xG, yG, zG);
    int4 pos = (int4)(xG, yG, zG, 0);

    const int x = get_local_id(0);
    const int y = get_local_id(1);
    const int z = get_local_id(2);

    //LOCAL OFFSET?
    if (xG >= VOL_SIZE_X || yG >= VOL_SIZE_Y || zG >= VOL_SIZE_Z){
        return;
    }
    int locGlobal = (xG + yG*VOL_SIZE_X + zG*VOL_SIZE_XtY); //Component later
    //float voxelValue = 0.0f; // readAndNormalize(volume, locGlobal, 0.0f);
    float voxelValue = readAndNormalize(volume, locGlobal, -1.0f);

    for (int addX = 0; (x + addX) < LSIZE_MEM_X && (xG + addX) < VOL_SIZE_X; addX += LSIZE_X){
        for (int addY = 0; (y + addY) < LSIZE_MEM_Y && (yG + addY) < VOL_SIZE_Y; addY += LSIZE_Y){
            for (int addZ = 0; (z + addZ) < LSIZE_MEM_Z && (zG + addZ) < VOL_SIZE_Z; addZ += LSIZE_Z){
                int locGlobal = ((xG + addX) + (yG + addY)*VOL_SIZE_X + (zG + addZ)*VOL_SIZE_XtY);
                float val = readAndNormalize(volume, locGlobal, -1.0f);
                setLocalValue(sharedMem, (x + addX), (y + addY), (z + addZ), val);
            }
        }
    }

    /*
    if (false){*/
    // WE ARE ASSUMING X workgroup is big enough to load all inputdata in 2 runs. 
    //                 the rest need loops incase workgroup is too small in that direction.
    /*
    //READ A and normalize
    int AaddX = 0; int AaddY = 0; int AaddZ = 0;
    {
        int locGlobalA = ((xG + AaddX) + (yG + AaddY)*VOL_SIZE_X + (zG + AaddZ)*VOL_SIZE_XtY); //Component later
        float valA = readAndNormalize(volume, locGlobalA, -1.0f);
        setLocalValue(sharedMem, (x + AaddX), (y + AaddY), (z + AaddZ), valA);
    }
    
    //barrier(CLK_LOCAL_MEM_FENCE);
    //READ B and normalize
    int BaddX = 0; int BaddZ = 0;
    for (int BaddY = LSIZE_Y; (y + BaddY) < LSIZE_MEM_Y && (yG + BaddY) < VOL_SIZE_Y; BaddY += LSIZE_Y){
    //int BaddX = 0; int BaddY = LSIZE_Y; int BaddZ = 0;
    //if ((y + BaddY) < LSIZE_MEM_Y && (yG + BaddY) < VOL_SIZE_Y){
        int locGlobalB = ((xG + BaddX) + (yG + BaddY)*VOL_SIZE_X + (zG + BaddZ)*VOL_SIZE_XtY); //Component later
        float valB = readAndNormalize(volume, locGlobalB, -1.0f);
        setLocalValue(sharedMem, (x + BaddX), (y + BaddY), (z + BaddZ), valB);
    }
    //barrier(CLK_LOCAL_MEM_FENCE);
    //READ C and normalize
    int CaddX = LSIZE_X; int CaddY = 0; int CaddZ = 0;
    if ((x + CaddX) < LSIZE_MEM_X && (xG + CaddX) < VOL_SIZE_X){
        int locGlobalC = ((xG + CaddX) + (yG + CaddY)*VOL_SIZE_X + (zG + CaddZ)*VOL_SIZE_XtY); //Component later
        float valC = readAndNormalize(volume, locGlobalC, -1.0f);
        setLocalValue(sharedMem, (x + CaddX), (y + CaddY), (z + CaddZ), valC);
    }
    //barrier(CLK_LOCAL_MEM_FENCE);
    //READ D and normalize
    int DaddX = LSIZE_X; int DaddZ = 0;
    for (int DaddY = LSIZE_Y; (x + DaddX) < LSIZE_MEM_X && (xG + DaddX) < VOL_SIZE_X
        && (y + DaddY) < LSIZE_MEM_Y && (yG + DaddY) < VOL_SIZE_Y; DaddY += LSIZE_Y){
    //int DaddX = LSIZE_X; int DaddY = LSIZE_Y; int DaddZ = 0;
    //if ((x + DaddX) < LSIZE_MEM_X && (xG + DaddX) < VOL_SIZE_X
    //&& (y + DaddY) < LSIZE_MEM_Y && (yG + DaddY) < VOL_SIZE_Y){
        int locGlobalD = ((xG + DaddX) + (yG + DaddY)*VOL_SIZE_X + (zG + DaddZ)*VOL_SIZE_XtY); //Component later
        float valD = readAndNormalize(volume, locGlobalD, -1.0f);
        setLocalValue(sharedMem, (x + DaddX), (y + DaddY), (z + DaddZ), valD);
    }
    //barrier(CLK_LOCAL_MEM_FENCE);
    int addZ = LSIZE_Z;
    for (int addZ = LSIZE_Z; (z + addZ) < LSIZE_MEM_Z && (zG + addZ) < VOL_SIZE_Z; addZ += LSIZE_Z){
    //if ((z + addZ) < LSIZE_MEM_Z && (zG + addZ) < VOL_SIZE_Z){
        //READ E and normalize
        int EaddX = 0; int EaddY = 0;
        {
            int locGlobalE = ((xG + EaddX) + (yG + EaddY)*VOL_SIZE_X + (zG + addZ)*VOL_SIZE_XtY); //Component later
            float valE = readAndNormalize(volume, locGlobalE, -1.0f);
            setLocalValue(sharedMem, (x + EaddX), (y + EaddY), (z + addZ), valE);
        }
        //MEMBLOCK?
        //READ G and normalize
        int GaddX = 0;
        for (int GaddY = LSIZE_Y; (y + GaddY) < LSIZE_MEM_Y && (yG + GaddY) < VOL_SIZE_Y; GaddY += LSIZE_Y){
        //int GaddX = 0; int GaddY = LSIZE_Y;
        //if ((y + GaddY) < LSIZE_MEM_Y && (yG + GaddY) < VOL_SIZE_Y){
            int locGlobalG = ((xG + GaddX) + (yG + GaddY)*VOL_SIZE_X + (zG + addZ)*VOL_SIZE_XtY); //Component later
            float valG = readAndNormalize(volume, locGlobalG, -1.0f);
            setLocalValue(sharedMem, (x + GaddX), (y + GaddY), (z + addZ), valG);
        }
        //MEMBLOCK?
        //READ F and normalize
        int FaddX = LSIZE_X; int FaddY = 0;
        if ((x + FaddX) < LSIZE_MEM_X && (xG + FaddX) < VOL_SIZE_X){
            int locGlobalF = ((xG + FaddX) + (yG + FaddY)*VOL_SIZE_X + (zG + addZ)*VOL_SIZE_XtY); //Component later
            float valF = readAndNormalize(volume, locGlobalF, -1.0f);
            setLocalValue(sharedMem, (x + FaddX), (y + FaddY), (z + addZ), valF);
        }
        //MEMBLOCK?
        //READ H and normalize
        int HaddX = LSIZE_X;
        for (int HaddY = LSIZE_Y; (x + HaddX) < LSIZE_MEM_X && (xG + HaddX) < VOL_SIZE_X
            && (y + HaddY) < LSIZE_MEM_Y && (yG + HaddY) < VOL_SIZE_Y; HaddY += LSIZE_Y){
        //int HaddX = LSIZE_X; int HaddY = LSIZE_Y;
        //if ((x + HaddX) < LSIZE_MEM_X && (xG + HaddX) < VOL_SIZE_X
        //&& (y + HaddY) < LSIZE_MEM_Y && (yG + HaddY) < VOL_SIZE_Y){
            int locGlobalH = ((xG + HaddX) + (yG + HaddY)*VOL_SIZE_X + (zG + addZ)*VOL_SIZE_XtY); //Component later
            float valH = readAndNormalize(volume, locGlobalH, -1.0f);
            setLocalValue(sharedMem, (x + HaddX), (y + HaddY), (z + addZ), valH);
        }
    }
    /*}*
    */

    
    
    barrier(CLK_LOCAL_MEM_FENCE);
    /*
    float accumulationValue = 0.0f;
    int counter = 0;
    for (int halfWidth = 0; halfWidth <= HALF_WIDTH; halfWidth++){

    }
    */
    int locLocal = (x + y*LSIZE_MEM_X + z*LSIZE_MEM_XtY);
    voxelValue = sharedMem[locLocal];
    if (voxelValue < -0.5f){//true){
        //MEMBLOCK //REALLY IMPORTANT ONE
        //barrier(CLK_LOCAL_MEM_FENCE);
        //All data is read to local, perform calculation

        float accumulationValue = 0.0f;
        int counter = 0;
        
        #if PROGRESSIVE_PNN
            //int HW_boost = 0;
            //int halfWidth = 0;
            int HW_prog = 0;
            while ((counter == 0) && HW_prog <= HALF_WIDTH){
                HW_prog++;
            //while ((counter == 0) && (PROGRESSIVE_PNN || (HW_boost == 0)) && (HW_boost < 3)){
                //int minX = xG - HALF_WIDTH - HW_boost; //or max this and 0? or sampler handles it?
                int minX = max2i((x - HW_prog), 0); //x; //or max this and 0? or sampler handles it?
                //int minY = yG - HALF_WIDTH - HW_boost;
                int minY = max2i((y - HW_prog), 0); //yG - HALF_WIDTH - HW_boost; //y;
                //int minZ = zG - HALF_WIDTH - HW_boost;
                int minZ = max2i((z - HW_prog), 0); //zG - HALF_WIDTH - HW_boost; //z;
                int maxX = x + HW_prog;
                maxX = min2i(maxX, ((int)VOL_SIZE_X - 1));
                int maxY = y + HW_prog;
                maxY = min2i(maxY, ((int)VOL_SIZE_Y - 1)); // minY + HALF_WIDTH_X2 + (2 * HW_boost);
                int maxZ = z + HW_prog;
                maxZ = min2i(maxZ, ((int)VOL_SIZE_Z - 1)); //minZ + HALF_WIDTH_X2 + (2 * HW_boost);

                //Can restructure to avoid overlap!
                //  Starts at 0 HW and adds up to HALF_WIDTH(max) ie. 0,1,2,3 for GridSize 7
                // 0 means checking the node itself
                for (int xi = minX; xi <= maxX; xi++){
                    for (int yi = minY; yi <= maxY; yi++){
                        for (int zi = minZ; zi <= maxZ; zi++){
                            if (xi == x && yi == y && zi == z){ continue; }
                            int loc = (xi + yi*LSIZE_MEM_X + zi*LSIZE_MEM_XtY);
                            float locValue = sharedMem[loc];
                            //int loc = (xi + yi*VOL_SIZE_X + zi*VOL_SIZE_XtY);
                            //float locValue = readAndNormalize(volume, loc, -1.0f);
                            if (locValue >= -0.5f){ //ev > -0.5? for inaccuracy?
                                accumulationValue += locValue;
                                counter++;
                            }
                        }
                    }
                }
                //HW_boost++;
            }
        #else
            int minX = max2i((x - HALF_WIDTH), 0); 
            int minY = max2i((y - HALF_WIDTH), 0);
            int minZ = max2i((z - HALF_WIDTH), 0);
            int maxX = min2i(x + HALF_WIDTH, ((int)VOL_SIZE_X - 1));
            int maxY = min2i(y + HALF_WIDTH, ((int)VOL_SIZE_Y - 1));
            int maxZ = min2i(z + HALF_WIDTH, ((int)VOL_SIZE_Z - 1));
            for (int xi = minX; xi <= maxX; xi++){
                for (int yi = minY; yi <= maxY; yi++){
                    for (int zi = minZ; zi <= maxZ; zi++){
                        if (xi == x && yi == y && zi == z){ continue; }
                        int loc = (xi + yi*LSIZE_MEM_X + zi*LSIZE_MEM_XtY);
                        float locValue = sharedMem[loc];
                        if (locValue >= -0.5f){ //ev > -0.5? for inaccuracy?
                            accumulationValue += locValue;
                            counter++;
                        }
                    }
                }
            }
        #endif
        
       

        //float 
        if (counter != 0 && accumulationValue >= 0.0f){
            voxelValue = accumulationValue / counter;
        }
        else {
            voxelValue = 0.0f;
        }

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