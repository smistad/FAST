__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

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

void accumulateValuesInVolume(__global float * volume, __global int * semaphor, int3 volumePoint, float p, float w){
    //volAccess available from Us3Dhybrid as ImageAccess::pointer
    //barrier(CLK_GLOBAL_MEM_FENCE);//TODO add a lock to these????
    int loc = volumePoint.x + volumePoint.y*VOL_SIZE_X + volumePoint.z*VOL_SIZE_X*VOL_SIZE_Y;
    GetSemaphor(&semaphor[loc]);
    {
        /*
        //component + pos.x*VOL_SIZE_X * 2 + pos.y*VOL_SIZE_X*VOL_SIZE_Y * 2 + pos.z*VOL_SIZE_X*VOL_SIZE_Y*VOL_SIZE_Z * 2
        float oldP = volume[loc * 2];//get3DvolumeVectorValue(volume, volumePoint, 0);
        //0.0f;// get3DvolumeValue(volume, volumePoint, 0, outputDataType);
        float oldW = volume[loc * 2 + 1];//get3DvolumeVectorValue(volume, volumePoint, 1);
        //0.0f;//get3DvolumeValue(volume, volumePoint, 1, outputDataType);
        if (oldP < 0.0f) oldP = 0.0f;
        if (oldW < 0.0f) oldW = 0.0f;
        float newP = oldP + p*w;
        float newW = oldW + w;
        */
        volume[loc * 2] = p;// newP;
        volume[loc * 2 + 1] = w;// newW;
        //set3DvolumeVectorValue(volume, volumePoint, newP, 0);// , outputDataType);
        //set3DvolumeVectorValue(volume, volumePoint, newW, 1);// , outputDataType);
    }
    ReleaseSemaphor(&semaphor[loc]);
}

__kernel void addToVolume(
    
    __global float* volume,
    __global int* semaphor,
    __private const float add
    ){
    //__read_only image2d_t frame,
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    //int dataType = get_image_channel_data_type(frame);

    int3 volumePoint = (int3)(x, y, z);
    float p = add;// getPixelValue(frame, intersectionPointLocal, dataType); //TODO FIX
    float w = 1;
    accumulateValuesInVolume(volume, semaphor, volumePoint, p, w);
}