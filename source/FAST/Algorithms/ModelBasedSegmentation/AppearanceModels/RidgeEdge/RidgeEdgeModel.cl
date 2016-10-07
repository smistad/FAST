__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

float3 transform(const float3 position, __constant float* transformationMatrix) {
    float3 result;
    result.x = position.x*transformationMatrix[0] + position.y*transformationMatrix[1] + position.z*transformationMatrix[2] + transformationMatrix[3];
    result.y = position.x*transformationMatrix[4] + position.y*transformationMatrix[5] + position.z*transformationMatrix[6] + transformationMatrix[7];
    result.z = position.x*transformationMatrix[8] + position.y*transformationMatrix[9] + position.z*transformationMatrix[10] + transformationMatrix[11];
    return result;
}


__kernel void edgeDetection2D(
        __read_only image2d_t image,
        __global float* points,
        __private float lineSearchDistance,
        __private float sampleSpacing,
        __local float* intensityProfile,
        __global float* results,
        __private float spacingX,
        __private float spacingY,
        __private int size
        ) {
    const int sampleNr = get_global_id(0);
    const int pointNr = get_global_id(1);
    const int nrOfSamples = get_global_size(0);
    const float2 position = vload2(pointNr*2, points);
    const float2 normal = vload2(pointNr*2+1, points);
    const float distance = -lineSearchDistance/2.0f + sampleNr*sampleSpacing;
    //printf("s: %d, d: %f \n", sampleNr, distance);
    float2 samplePosition = position + distance*normal;
    samplePosition.x /= spacingX;
    samplePosition.y /= spacingY;
    //printf("p: %f %f, sp: %f %f, n: %f %f\n", position.x, position.y, samplePosition.x, samplePosition.y, normal.x, normal.y);

    // All read intensity profile into local memory
    // TODO support for other types of images
    intensityProfile[sampleNr] = (float)read_imageui(image, sampler, samplePosition).x;

    // TODO how do we deal with out of image/mask here?

    // Synchronize
    barrier(CLK_LOCAL_MEM_FENCE);

    float sumBeforeRidge = 0.0f;
    float sumInRidge = 0.0f;
    float sumAfterRidge = 0.0f;
    int startPos = -1;
    int endPos = -1;
    for(int i = 0; i < nrOfSamples; i++) {
        if(startPos == -1 && intensityProfile[i] > 0)
            startPos = i;
        if(startPos >= 0 && intensityProfile[i] == 0) {
            endPos = i;
        }
        if(startPos >= 0 && endPos == -1) {
            if(i <= sampleNr) {
                sumBeforeRidge += intensityProfile[i];
            } else if(i <= sampleNr + size) {
                sumInRidge += intensityProfile[i];
            }
        }
    }
    if(endPos < 0)
        endPos = nrOfSamples - 1;

    // Calculate intensity diffs
    const float averageBeforeRidge = sumBeforeRidge / (sampleNr+1 - startPos);
    const float averageInRidge = sumInRidge / size;

    const float intensityDifference = averageInRidge - averageBeforeRidge;

    results[sampleNr + pointNr*nrOfSamples] = intensityDifference;


    // TODO

    // Store score and uncertainty in local memory
    // Use first thread to calculate best score of all samples
    // Store sampleNr and uncertainty in global memory

}
