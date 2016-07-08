__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

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






/*
void Us3Dhybrid::generateOutputVolume(){
    volAccess = AccumulationVolume->getImageAccess(accessType::ACCESS_READ_WRITE);
    std::cout << "Final reconstruction calculations!" << std::endl;
    // Finally, calculate reconstructed volume
    output = getStaticOutputData<Image>(0);
    output->create(AccumulationVolume->getSize(), AccumulationVolume->getDataType(), 1); //1-channeled output volume
    ImageAccess::pointer outAccess = output->getImageAccess(ACCESS_READ_WRITE);
    for (int x = 0; x < output->getWidth(); x++){
        std::cout << ".";
        for (int y = 0; y < output->getHeight(); y++){
            for (int z = 0; z < output->getDepth(); z++){
                Vector3i location = Vector3i(x, y, z);
                float p = volAccess->getScalar(location, 0);
                float w = volAccess->getScalar(location, 1);
                if (w > 0.0 && p >= 0.0){ // w != 0.0 to avoid division error // This other logic to avoid uninitialized voxels
                    float finalP = p / w;
                    outAccess->setScalar(location, finalP, 0);
                }
                else{
                    outAccess->setScalar(location, 0.0, 0);
                }
            }
        }
    }
    outAccess.release();
    std::cout << "\nDONE calculations!" << std::endl;
    //Can possibly make 2D slices here or alternatively to the one above
}*/

/*
__kernel void normalizeVolume(
__read_only image3d_t input,
__write_only image3d_t output
)
{//Sizes?
const int3 pos = { get_global_id(0), get_global_id(1), get_global_id(2)}; //x, y, z, component
int dataType = get_image_channel_data_type(input);

float p = 0.0f;
float w = 0.0f;
if (dataType == CLK_FLOAT) {
float2 val = read_imagef(input, sampler, pos);
p = val.x; //ETC?
w = val.y;
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
*/

/*
    __read_only image2d_t input,
    __constant float * mask,
    __write_only image2d_t output,
    __private unsigned char maskSize
    ) {

    const int2 pos = { get_global_id(0), get_global_id(1) };
    const unsigned char halfSize = (maskSize - 1) / 2;

    float sum = 0.0f;
    int dataType = get_image_channel_data_type(input);
    for (int x = -halfSize; x <= halfSize; x++) {
        for (int y = -halfSize; y <= halfSize; y++) {
            const int2 offset = { x, y };
            if (dataType == CLK_FLOAT) {
                sum += mask[x + halfSize + (y + halfSize)*maskSize] * read_imagef(input, sampler, pos + offset).x;
            }
            else if (dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
                sum += mask[x + halfSize + (y + halfSize)*maskSize] * read_imageui(input, sampler, pos + offset).x;
            }
            else {
                sum += mask[x + halfSize + (y + halfSize)*maskSize] * read_imagei(input, sampler, pos + offset).x;
            }
        }
    }

    //sum += STATIC_ADD;
    sum = fabs(sum);

    int outputDataType = get_image_channel_data_type(output);
    if (outputDataType == CLK_FLOAT) {
        write_imagef(output, pos, sum);
    }
    else if (outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(output, pos, round(sum));
    }
    else {
        write_imagei(output, pos, round(sum));
    }
    }*/