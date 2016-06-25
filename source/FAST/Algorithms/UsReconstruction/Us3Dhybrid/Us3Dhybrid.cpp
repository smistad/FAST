#include "FAST/Algorithms/UsReconstruction/Us3Dhybrid/Us3Dhybrid.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"

//For show
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/TestDataPath.hpp"

using namespace fast;

void Us3Dhybrid::setOutputType(DataType type){
    mOutputType = type;
    mOutputTypeSet = true;
    mIsModified = true;
}

bool Us3Dhybrid::hasCalculatedVolume(){
    return volumeCalculated;
}

void Us3Dhybrid::setScaleToMax(float scaleToMax){
    mScaleToMax = scaleToMax;
    mIsModified = true;
}

void Us3Dhybrid::setVoxelSpacing(float voxelSpacing){
    mVoxelSpacing = voxelSpacing;
    //dv = 0.1f / (mVoxelSpacing*globalScalingValue);
    volumeCalculated = false;
    volumeInitialized = false;
    mIsModified = true;
}

void Us3Dhybrid::setDV(float setdv){
    dv = setdv;
    volumeCalculated = false;
    volumeInitialized = false;
    mIsModified = true;
}

void Us3Dhybrid::setRmax(float maxRvalue){
    Rmax = maxRvalue;
    volumeCalculated = false;
    mIsModified = true;
}

void Us3Dhybrid::setGlobalScaling(float globalScaling){
    float oldScaling = globalScalingValue;
    globalScalingValue = globalScaling;
    dv = dv * (oldScaling / globalScalingValue);
    //Rmax = Rmax * (oldScaling / globalScalingValue);
    volumeCalculated = false;
    volumeInitialized = false;
    mIsModified = true;
}

void Us3Dhybrid::setZDirInitSpacing(float zInitSpacing){
    zDirInitSpacing = zInitSpacing;
    volumeCalculated = false;
    volumeInitialized = false;
    mIsModified = true;
}

void Us3Dhybrid::setPNNrunMode(bool pnnRunMode){
    runAsPNNonly = pnnRunMode;
    volumeCalculated = false;
    volumeInitialized = false;
    mIsModified = true;
}

void Us3Dhybrid::setVNNrunMode(bool vnnRunMode){
    runAsVNNonly = vnnRunMode;
    volumeCalculated = false;
    volumeInitialized = false;
    mIsModified = true;
}

void Us3Dhybrid::setCLrun(bool clRunMode){
    runCLhybrid = clRunMode;
    volumeCalculated = false;
    volumeInitialized = false;
    mIsModified = true;
}

Us3Dhybrid::Us3Dhybrid(){
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_STATIC, 0); //enum OutputDataType { OUTPUT_STATIC, OUTPUT_DYNAMIC, OUTPUT_DEPENDS_ON_INPUT };
    //createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0); //OUTPUT_DEPENDS_ON_INPUT necessary? TODO
    
    //create openCL prog here
    //--//createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter2D.cl", "2D");
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/UsReconstruction/Us3Dhybrid/us3Dhybrid.cl", "us3Dhybrid");
    //--// store different compiled for settings (dimension/variables...)
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/UsReconstruction/Us3Dhybrid/normalizeVolume.cl", "normalizeVolume");

    mIsModified = true; // needed?
    mOutputTypeSet = false;

    //volume;
    dv = 1.0;
    globalScalingValue = 1.0f;
    zDirInitSpacing = 0.02f; // 0.02f; // 0.1f //set spacing(2) = 0.1 eller noe? er default 1.0
    setVoxelSpacing(0.1f);
    Rmax = 5.0; //2?
    mScaleToMax = 100.0f;
    volumeCalculated = false;
    volumeInitialized = false;
    firstFrameSet = false;
    reachedEndOfStream = false;
    frameList = {};
    iterartorCounter = 0;
    runAsPNNonly = false;
    runAsVNNonly = false;
    runCLhybrid = false;
}

Us3Dhybrid::~Us3Dhybrid(){
    //delete something
}

void Us3Dhybrid::waitToFinish() {
    if (!getMainDevice()->isHost()) {
        OpenCLDevice::pointer device = getMainDevice();
        device->getCommandQueue().finish();
    }
}

void Us3Dhybrid::accumulateValuesInVolumeData(Vector3i volumePoint, float p, float w){
    //volData available from Us3Dhybrid as float *
    int loc = (volumePoint.x() + volumePoint.y()*yLocMultiplier + volumePoint.z()*zLocMultiplier) * 2;
    float oldP = volData[loc];
    float oldW = volData[loc + 1];
    if (oldP < 0.0f) oldP = 0.0f;
    if (oldW < 0.0f) oldW = 0.0f;
    float newP = oldP + p*w;
    float newW = oldW + w;
    volData[loc] = newP;
    volData[loc + 1] = newW;
}

void Us3Dhybrid::accumulateValuesInVolume(Vector3i volumePoint, float p, float w){
    //volAccess available from Us3Dhybrid as ImageAccess::pointer
    float oldP = volAccess->getScalar(volumePoint, 0); //out of bounds????
    float oldW = volAccess->getScalar(volumePoint, 1);
    if (oldP < 0.0f) oldP = 0.0f;
    if (oldW < 0.0f) oldW = 0.0f;
    float newP = oldP + p*w;
    float newW = oldW + w;
    volAccess->setScalar(volumePoint, newP, 0);
    volAccess->setScalar(volumePoint, newW, 1);
}

void Us3Dhybrid::addTransformationToFrame(Image::pointer frame, AffineTransformation::pointer addTransformation){
    //adds a transformation to the existing transformation on frame
    //Might just be a translation
    AffineTransformation::pointer oldImgTransform = SceneGraph::getAffineTransformationFromData(frame);
    //AffineTransformation::pointer newImgTransform = oldImgTransform->multiply(addTransformation);
    Matrix4f transformMatrix = addTransformation->matrix();
    Vector4f t0 = transformMatrix.row(0);
    Vector4f t1 = transformMatrix.row(1);
    Vector4f t2 = transformMatrix.row(2);
    Vector4f t3 = transformMatrix.row(3);
    Matrix4f thisMatrix = oldImgTransform->matrix();
    Vector4f i0 = thisMatrix.row(0);
    Vector4f i1 = thisMatrix.row(1);
    Vector4f i2 = thisMatrix.row(2);
    Vector4f i3 = thisMatrix.row(3);
    AffineTransformation::pointer newImgTransform = addTransformation->multiply(oldImgTransform);
    //ev?
    //AffineTransformation::pointer newImgTransform = oldImgTransform + addTransformation;
    frame->getSceneGraphNode()->setTransformation(newImgTransform);
}

Vector2i Us3Dhybrid::getFrameRangeInVolume(int frameNr, int domDir, int dir){//Image::pointer frame, int domDir, int dir){
    //domDir of x,y,z and dir of a,b
    Vector2i outputRange;
    //If square just use corners //TODO implement alternative? Or done in calculation of min/max lists?
    Vector3f minCoords = frameMinList[frameNr];
    Vector3f maxCoords = frameMaxList[frameNr];
    //TODO implement or fetch minmax
    if ((domDir == 0 && dir == 0) || (domDir == 2 && dir == 1)){
        //If domDir:x want a-dir or if domDir:z want b-dir
        //We are returning range from the Y-axis
        float minimum = minCoords(1); // Y
        float maximum = maxCoords(1); // Y
        //outputRange = Vector2i(floor(minimum), ceil(maximum)); //TODO eller snu om på floor/ceil for å begrense det innenfor frame mer?
        outputRange = Vector2i(round(minimum), round(maximum));
        //outputRange = Vector2i(ceil(minimum), floor(maximum));
    }
    else if ((domDir == 1 && dir == 0) || (domDir == 2 && dir == 0)){
        //If domDir:y or domDir:z and want a-dir
        //We are returning range from the X-axis
        float minimum = minCoords(0); // X
        float maximum = maxCoords(0); // X
        //outputRange = Vector2i(floor(minimum), ceil(maximum));
        outputRange = Vector2i(round(minimum), round(maximum));
    }
    else if ((domDir == 0 && dir == 1) || (domDir == 1 && dir == 1)){
        //If domDir:x or domDir:y and want b-dir
        //We are returning range from the Z-axis
        float minimum = minCoords(2); // Z
        float maximum = maxCoords(2); // Z
        //outputRange = Vector2i(floor(minimum), ceil(maximum));
        outputRange = Vector2i(round(minimum), round(maximum));
    }
    return outputRange;
}

AffineTransformation::pointer Us3Dhybrid::getInverseTransformation(Image::pointer frame){
    AffineTransformation::pointer imageTransformation = SceneGraph::getAffineTransformationFromData(frame);
    Matrix4f transformMatrix = imageTransformation->matrix();
    Vector4f t0 = transformMatrix.row(0);
    Vector4f t1 = transformMatrix.row(1);
    Vector4f t2 = transformMatrix.row(2);
    Vector4f t3 = transformMatrix.row(3);
    AffineTransformation::pointer inverseTransformation = imageTransformation->getInverse();
    Matrix4f inverseTransformMatrix = inverseTransformation->matrix();
    Vector4f i0 = inverseTransformMatrix.row(0);
    Vector4f i1 = inverseTransformMatrix.row(1);
    Vector4f i2 = inverseTransformMatrix.row(2);
    Vector4f i3 = inverseTransformMatrix.row(3);
    return inverseTransformation;
}

float Us3Dhybrid::getPixelValueData(Vector3f point){
    //Gets frameAccess from Us3Dhybrid class
    float x = point(0);
    float y = point(1);
    int z = round(point(2));
    int xCeil = ceil(x);
    int yCeil = ceil(y);
    if (xCeil < 0 || yCeil < 0 || z != 0 || xCeil > frameSize.x()-1 || yCeil > frameSize.y()-1){
        //Throw error? Should not need to occur if appropriate bufferXY in last function
        return 0.0f;
    }
    /*

    //using TYPE_UINT8(1)
    defined by: fastCaseTypeMacro(TYPE_UINT8, uchar, call)
    if(position.x() < 0 || position.y() < 0 || position.z() < 0 ||
            position.x() > size.x()-1 || position.y() > size.y()-1 || position.z() > size.z()-1 || channel >= image->getNrOfComponents())
        throw OutOfBoundsException();

        T value = data[(position.x() + position.y()*size.x() + position.z()*size.x()*size.y())*image->getNrOfComponents() + channel];

        float floatValue;
        if(image->getDataType() == TYPE_SNORM_INT16) {
        floatValue = std::max(-1.0f, (float)value / 32767.0f);
        } else if(image->getDataType() == TYPE_UNORM_INT16) {
        floatValue = (float)value / 65535.0f;
        } else {
        floatValue = value;
        }

        return floatValue;
    */
    int sizeX = frameSize.x();
    int sizeY = frameSize.y();
    int yMod = xCeil + yCeil * frameSize.x();
    int yMod2 = xCeil + yCeil * sizeX;
    int chan = frameChannels;
    DataType type = frameType;

    int xFloor = floor(x);
    int yFloor = floor(y);
    if (xFloor < 0){
        int loc = (xCeil + yCeil*frameSize.x() + z*frameSize.x()*frameSize.y())*firstFrame->getNrOfComponents() + 0;// channel;
        int locMaxMax = (xCeil + yCeil * frameSize.x())*3;
        uchar pMaxMax = frameData[locMaxMax];
        //float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        if (yFloor < 0){
            //Case 1
            return (float)pMaxMax;
        }
        else {
            //Case 3
            int locMaxMin = (xCeil + yFloor * frameSize.x())*3;
            uchar pMaxMin = frameData[locMaxMin];
            //float pMaxMin = frameAccess->getScalar(Vector3i(xCeil, yFloor, z));
            float v = y - yFloor;
            float pRight = ((float)pMaxMin)*(1 - v) + ((float)pMaxMax)*v;
            return pRight;
        }
    }
    else if (yFloor < 0){
        //Case 2
        float u = x - xFloor;
        int locMinMax = (xFloor + yCeil * frameSize.x()) * 3;
        int locMaxMax = (xCeil + yCeil * frameSize.x()) * 3;
        uchar pMinMax = frameData[locMinMax];
        uchar pMaxMax = frameData[locMaxMax];
        //float pMinMax = frameAccess->getScalar(Vector3i(xFloor, yCeil, z));
        //float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        float pBot = ((float)pMinMax)*(1 - u) + ((float)pMaxMax)*u;
        return pBot;
    }
    else {
        int loc = (xFloor + yFloor*frameSize.x() + z*frameSize.x()*frameSize.y())*firstFrame->getNrOfComponents() + 0;// channel;
        int locMinMin = (xFloor + yFloor * frameSize.x()) * 3;
        int locMinMax = (xFloor + yCeil * frameSize.x()) * 3;
        int locMaxMin = (xCeil + yFloor * frameSize.x()) * 3;
        int locMaxMax = (xCeil + yCeil * frameSize.x()) * 3;
        uchar pMinMin = frameData[locMinMin];
        uchar pMinMax = frameData[locMinMax];
        uchar pMaxMin = frameData[locMaxMin];
        uchar pMaxMax = frameData[locMaxMax];
        /*
        float pMinMin = frameAccess->getScalar(Vector3i(xFloor, yFloor, z)); //ev 0 for point(2) or round?
        float pMinMax = frameAccess->getScalar(Vector3i(xFloor, yCeil, z));
        float pMaxMin = frameAccess->getScalar(Vector3i(xCeil, yFloor, z));
        float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        */
        //Calculate horizontal interpolation
        float u = point(0) - floor(point(0));
        float pTop = ((float)pMinMin)*(1 - u) + ((float)pMaxMin)*u;
        float pBot = ((float)pMinMax)*(1 - u) + ((float)pMaxMax)*u;
        //Calculate final vertical interpolation
        float v = point(1) - floor(point(1));
        float pValue = pTop*(1 - v) + pBot*v;//pBot*(1 - v) + pTop*v;
        return pValue;
    }
}

//float p = getPixelValue(frameAccess, intersectionPointLocal);
float Us3Dhybrid::getPixelValue(Vector3f point){ 
    //Gets frameAccess from Us3Dhybrid class
    float x = point(0);
    float y = point(1);
    int z = round(point(2));
    int xCeil = ceil(x);
    int yCeil = ceil(y);
    if (xCeil < 0 || yCeil < 0 || z != 0 || xCeil >= frameSize.x() || yCeil >= frameSize.y()){
        //Throw error? Should not need to occur if appropriate bufferXY in last function
        return 0.0f;
    }
    int xFloor = floor(x);
    int yFloor = floor(y);
    if (xFloor < 0){
        float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        if (yFloor < 0){
            //Case 1
            return pMaxMax;
        }
        else {
            //Case 3
            float pMaxMin = frameAccess->getScalar(Vector3i(xCeil, yFloor, z));
            float v = y - yFloor;
            float pRight = pMaxMin*(1 - v) + pMaxMax*v;
            return pRight;
        }
    }
    else if (yFloor < 0){
        //Case 2
        float u = x - xFloor;
        float pMinMax = frameAccess->getScalar(Vector3i(xFloor, yCeil, z));
        float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        float pBot = pMinMax*(1 - u) + pMaxMax*u;
        return pBot;
    }
    else {
        float pMinMin = frameAccess->getScalar(Vector3i(xFloor, yFloor, z)); //ev 0 for point(2) or round?
        float pMinMax = frameAccess->getScalar(Vector3i(xFloor, yCeil, z));
        float pMaxMin = frameAccess->getScalar(Vector3i(xCeil, yFloor, z));
        float pMaxMax = frameAccess->getScalar(Vector3i(xCeil, yCeil, z));
        //Calculate horizontal interpolation
        float u = point(0) - floor(point(0));
        float pTop = pMinMin*(1 - u) + pMaxMin*u;
        float pBot = pMinMax*(1 - u) + pMaxMax*u;
        //Calculate final vertical interpolation
        float v = point(1) - floor(point(1));
        float pValue = pTop*(1 - v) + pBot*v;//pBot*(1 - v) + pTop*v;
        return pValue;
    }  
}

// ##### ##### Other functions ##### ##### //

// TODO implement
AffineTransformation::pointer getTranslationFromMinCoordsVector(Vector3f minCoords){ //eller Us3Dhybrid::
    //get a transformation of translation -(minCoords) (ie. -(-10) = +10)
    Eigen::Translation3f translation = Eigen::Translation3f(-minCoords(0), -minCoords(1), -minCoords(2));
    AffineTransformation::pointer output = AffineTransformation::New();
    output->matrix() = Eigen::Affine3f(translation).matrix();
    return output;
}

AffineTransformation::pointer getScalingFromVector(Vector3f scale){
    //get a scaling transformation along x, y, z dirs
    
    Eigen::Affine3f m;
    m = Eigen::Scaling(scale(0), scale(1), scale(2));
    AffineTransformation::pointer output = AffineTransformation::New();
    output->matrix() = m.matrix();
    return output;
}

// Vector3f intersectionPointWorld = getIntersectionOfPlane(volumePoint, distance, imagePlaneNormal);
Vector3f Us3Dhybrid::getIntersectionOfPlane(Vector3i startPoint, float distance, Vector3f normalVector){
    Vector3f startPointF = Vector3f(startPoint(0), startPoint(1), startPoint(2));
    Vector3f moveDist = Vector3f(normalVector*distance);
    //Vector3f moveDist = Vector3f(normalVector(0)*distance, normalVector(1)*distance, normalVector(2)*distance);
    Vector3f endPoint = startPointF + moveDist;
    //TODO add check towards target plane ?? "Project" into target plane?
    return endPoint;
}

//Vector3f intersectionPointLocal = getLocalIntersectionOfPlane(intersectionPointWorld, thisFrameInverseTransform);
Vector3f Us3Dhybrid::getLocalIntersectionOfPlane(Vector3f intersectionPointWorld, AffineTransformation::pointer frameInverseTransform){
    //Plocal = InverseTransform * Pglobal
    Vector3f intersectionPointLocal = frameInverseTransform->multiply(intersectionPointWorld);
    return intersectionPointLocal;
}

bool Us3Dhybrid::isWithinFrame(Vector3f intersectionPointLocal, Vector3ui frameSize, float bufferXY, float bufferZ){
    //Ev use untransformed boundingBox?
    if (fabs(intersectionPointLocal(2)) > bufferZ){ //If z too out of bounds //should not occure? Transformation error?
        float badZ = intersectionPointLocal(2);
        return false;
    }
    bool inside = true;
    for (int axis = 0; axis < 2; axis++){ 
        //For each axis X and Y
        float point = intersectionPointLocal(axis);
        uint size = frameSize(axis);
        if (point + bufferXY < 0.0){ //Bigger val than frame on this axis
            inside = false;
            break;
        }
        if (point - bufferXY > float(size-1)){ //Bigger val than frame on this axis //MAYBE remove bufferXY here? Or add plus
            inside = false;
            break;
        }
    }
    return inside;
}

bool Us3Dhybrid::volumePointOutsideVolume(Vector3i volumePoint, Vector3i volumeSize){
    for (int k = 0; k < 3; k++){
        int point = volumePoint(k);
        float size = volumeSize(k);
        if (point < 0 || point >= size){
            return true;
        }
    }
    return false;
}

//Seems good?

float Us3Dhybrid::calculatePlaneDvalue(Vector3f pointInPlane, Vector3f planeNormal){
    float planeDvalue = -(pointInPlane(0)*planeNormal(0) + pointInPlane(1)*planeNormal(1) + pointInPlane(2)*planeNormal(2));
    return planeDvalue;
}

Vector3f Us3Dhybrid::getBasePointInPlane(Vector3f rootPoint, Vector3f normal, float planeDvalue, int a, int b, int domDir){
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

float Us3Dhybrid::getPointDistanceAlongNormal(Vector3i A, Vector3f B, Vector3f normal){
    // |(B-A).dot(normal)|
    // TODO check maths is performed correctly
    //float distance = Vector3f((B - A).dot(normal)).norm();
    Vector3f Af = Vector3f(A(0), A(1), A(2));
    Vector3f diff = (B - Af);
    //Vector3f diff = Vector3f()
    float prod = diff.dot(normal);
    //float distance = fabs(prod); //fabs((B - A).dot(normal));
    return prod; // IKKE abs-val for now... Needs it further. distance;
}

/*
Get distance from point worldPoint to plane neighFrame along the imagePlaneNormal //TODO update doc
* point: Point in world space
* normal: Normal vector from worldPoint
* planePoint & planeNormal: Point and normal used to define a neighboring frame plane
# Return distance from point to plane along normal
*/
float Us3Dhybrid::getDistanceAlongNormal(Vector3f point, Vector3f normal, Vector3f planePoint, Vector3f planeNormal){
    //Should handle undefined planePoint and planeNormal TODO check
    if (planePoint.maxCoeff() < 0.0 || planePoint.maxCoeff() < 0.0){ // != nan){// .isZero() .hasNaN() .isEmpty() || planeNormal.isEmpty()){
        return 0.0f;
    }
    //P0 = planePoint
    //L0 = point in world
    //N = planeNormal
    //L = normal from point/origin
    float divisor = normal.dot(planeNormal);
    float dividend = (planePoint - point).dot(planeNormal);
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

Vector2i Us3Dhybrid::getDomDirRange(Vector3f basePoint, int domDir, float dfDom, Vector3i volumeSize){
    float rootC = basePoint(domDir);
    int domDirSize = volumeSize(domDir);
    int startC = std::max(0.0f, ceil(rootC - dfDom)); //std::max(0.0f, round(rootC - dfDom)); //std::max(0.0f, ceil(rootC - dfDom));
    int endC = std::min(floor(rootC + dfDom), float(domDirSize - 1)); //std::min(round(rootC + dfDom), float(domDirSize - 1)); //std::min(ceil(rootC + dfDom), float(domDirSize - 1));
    return Vector2i(startC, endC);
}

//GOOD
Vector3i Us3Dhybrid::getVolumePointLocation(int a, int b, int c, int domDir){
    int x, y, z;
    if (domDir == 0){ //domDir: x
        x = c;
        y = a;
        z = b;
    }
    else if (domDir == 1){ //domDir: y
        x = a;
        y = c;
        z = b;
    }
    else if (domDir == 2){ //domDir: z
        x = a;
        y = b;
        z = c;
    }
    return Vector3i(x, y, z);
}

float Us3Dhybrid::calculateHalfWidth(float d1, float d2, float dv, float Rmax){
    float furthestNeighbour = std::max(d1, d2);
    float maxTotal = std::max(furthestNeighbour, dv);// *globalScalingValue);
    float results = std::min(maxTotal, Rmax);// *globalScalingValue);
    return results;
}

int Us3Dhybrid::getDominatingVectorDirection(Vector3f v){
    float domVal = fabs(v(0));
    int domDir = 0;
    if (fabs(v(1)) > domVal){
        domVal = fabs(v(1));
        domDir = 1;
    }
    if (fabs(v(2)) > domVal){
        domVal = fabs(v(2));
        domDir = 2;
    }
    return domDir;
}

Vector3i Us3Dhybrid::getRoundedIntVector3f(Vector3f v){
    return Vector3i(round(v(0)), round(v(1)), round(v(2)));
}

Vector3f Us3Dhybrid::getImagePlaneNormal(Image::pointer frame){ //TODO test?
    AffineTransformation::pointer imageTransformation = SceneGraph::getAffineTransformationFromData(frame);
    Vector3f p0 = imageTransformation->multiply(Vector3f(0, 0, 0));
    Vector3f p1 = imageTransformation->multiply(Vector3f(1, 0, 0));
    Vector3f p2 = imageTransformation->multiply(Vector3f(0, 1, 0));
    Vector3f imagePlaneNormal = (p1 - p0).cross(p2 - p0);
    imagePlaneNormal.normalize();
    return imagePlaneNormal;
}

// ##### #####OpenCL helper functions ##### ##### //
cl_float16 Us3Dhybrid::transform4x4tofloat16(AffineTransformation::pointer imgTransform){
    Matrix4f transformMatrix = imgTransform->matrix();
    Vector4f m0 = transformMatrix.row(0);
    float m00 = m0(0);
    float m01 = m0(1);
    float m02 = m0(2);
    float m03 = m0(3);
    Vector4f m1 = transformMatrix.row(1);
    float m10 = m1(0);
    float m11 = m1(1);
    float m12 = m1(2);
    float m13 = m1(3);
    Vector4f m2 = transformMatrix.row(2);
    float m20 = m2(0);
    float m21 = m2(1);
    float m22 = m2(2);
    float m23 = m2(3);
    Vector4f m3 = transformMatrix.row(3);
    float m30 = m3(0);
    float m31 = m3(1);
    float m32 = m3(2);
    float m33 = m3(3);
    cl_float16 returnValue = {  m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33, };
    return returnValue;
}

// ##### ##### CORE functions ##### ##### //
void Us3Dhybrid::executeFramePNN(Image::pointer frame){
    uint width = frame->getWidth();
    uint height = frame->getHeight();
    frameAccess = frame->getImageAccess(ACCESS_READ);
    //float* frameValues = (float*) frameAccess->get();
    //uint nrOfComponents = frame->getNrOfComponents();
    for (uint x = 0; x < width; x++){
        for (uint y = 0; y < height; y++){
            Vector3f pos = Vector3f(x, y, 0);
            AffineTransformation::pointer imgTransform = SceneGraph::getAffineTransformationFromData(frame);
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
            /*/ volAccess available from Us3Dhybrid as ImageAccess::pointer
            float oldP = volAccess->getScalar(volumePoint, 0); //out of bounds????
            float oldW = volAccess->getScalar(volumePoint, 1);
            if (oldP < 0.0f) oldP = 0.0f;
            if (oldW < 0.0f) oldW = 0.0f;
            float newP = oldP + p*w;
            float newW = oldW + w;
            volAccess->setScalar(volumePoint, newP, 0);
            volAccess->setScalar(volumePoint, newW, 1);
            */
        }
    }
    frameAccess->release();
}

void Us3Dhybrid::executeVNN(){
    std::cout << "Final VNN reconstruction calculations!" << std::endl;
    // Finally, calculate reconstructed volume
    Vector3ui size = Vector3ui(volumeSize.x(), volumeSize.y(), volumeSize.z());
    setOutputType(firstFrame->getDataType());
    std::cout << "Step 1" << std::endl;
    outputVolume = getStaticOutputData<Image>(0);
    std::cout << "Step 2" << std::endl;
    outputVolume->create(size, mOutputType, 1); //1-channeled outputVolume
    std::cout << "Step 3" << std::endl;
    volAccess = outputVolume->getImageAccess(accessType::ACCESS_READ_WRITE);
    //float * outputData = (float*)volAccess->get();
    /*
    std::vector<ImageAccess::pointer> storedAccess = {};
    for (int frameNr = 0; frameNr < frameBaseCornerList.size(); frameNr++){
    Image::pointer frame = frameList[frameNr];
    ImageAccess::pointer frameAcc = frame->getImageAccess(ACCESS_READ);
    storedAccess.push_back(frameAcc);
    }
    */
    std::cout << "Step 4" << std::endl;
    /*
    //Preprossess frames and find rough locations where they are close to frame (sample every 10/20/50pixel ie.)
    int stepX = 50;
    int stepY = 50;
    int stepZ = 1;
    float threshold = 100.0f;

    std::vector<Vector3i> positions = {};

    for (uint z = 0; z < size.z(); z += stepZ){
    std::cout << "z: " << z << std::endl;
    for (uint y = 0; y < size.y(); y += stepY){
    //std::cout << ".";
    for (uint x = 0; x < size.x(); x += stepX){
    Vector3i pos = Vector3i(x, y, z);
    std::vector<int> frameNumbers = {};
    float closestDist = 1000.0f;
    int closestFrameNr = -1;
    float p = 0.0f;
    for (int frameNr = 0; frameNr < frameBaseCornerList.size(); frameNr++){
    //float Us3Dhybrid::getPointDistanceAlongNormal(Vector3i A, Vector3f B, Vector3f normal)
    Vector3f planePoint = frameBaseCornerList[frameNr];
    Vector3f planeNormal = framePlaneNormalList[frameNr];
    float dist = getPointDistanceAlongNormal(pos, planePoint, planeNormal);
    if (dist < closestDist){
    AffineTransformation::pointer frameInvTrans = frameInverseTransformList[frameNr];
    Image::pointer frame = frameList[frameNr];
    Vector3f crossWorld = getIntersectionOfPlane(pos, dist, planeNormal);
    Vector3f crossLocal = getLocalIntersectionOfPlane(crossWorld, frameInvTrans);
    if (isWithinFrame(crossLocal, frame->getSize(), 0.5f, 0.5f)){
    frameNumbers.push_back(frameNr);
    closestDist = dist;
    closestFrameNr = frameNr;
    //frameAccess = storedAccess[frameNr]; //
    frameAccess = frame->getImageAccess(ACCESS_READ);
    p = getPixelValue(crossLocal);
    }

    }
    }
    }
    }
    }
    */

    std::cout << "Step 5" << std::endl;

    std::cout << "Running with dv: " << dv << " and Rmax: " << Rmax << std::endl;

    for (uint x = 0; x < size.x(); x++){
        std::cout << "x: " << x << std::endl;
        for (uint y = 0; y < size.y(); y++){
            std::cout << ".";
            //Store closest?
            for (uint z = 0; z < size.z(); z++){
                Vector3i pos = Vector3i(x, y, z);
                Vector3f posF = Vector3f(x, y, z);
                // Find closest plane
                float closestDist = Rmax; // 5.0f;
                int closestFrameNr = -1;
                Vector3f closestCrossLocal = Vector3f(0, 0, 0);
                float p = 0.0f;
                for (int frameNr = 0; frameNr < frameBaseCornerList.size(); frameNr++){
                    /*std::vector<Vector3f> frameBaseCornerList;
                    std::vector<Vector3f> framePlaneNormalList;
                    std::vector<AffineTransformation::pointer> frameInverseTransformList;
                    std::vector<float> framePlaneDValueList;*/
                    //float Us3Dhybrid::getPointDistanceAlongNormal(Vector3i A, Vector3f B, Vector3f normal)
                    Vector3f planePoint = frameBaseCornerList[frameNr];
                    Vector3f planeNormal = framePlaneNormalList[frameNr];
                    float dist = getPointDistanceAlongNormal(pos, planePoint, planeNormal);
                    if (fabs(dist) < closestDist){
                        //std::cout << "Close frame nr: " << frameNr << std::endl;
                        AffineTransformation::pointer frameInvTrans = frameInverseTransformList[frameNr];
                        Image::pointer frame = frameList[frameNr];
                        Vector3f crossWorld = getIntersectionOfPlane(pos, dist, planeNormal);
                        Vector3f crossLocal = getLocalIntersectionOfPlane(crossWorld, frameInvTrans);
                        if (isWithinFrame(crossLocal, frame->getSize(), 0.5f, 0.5f)){
                            //std::cout << "Frame = Within" << std::endl;
                            closestDist = fabs(dist);
                            closestFrameNr = frameNr;
                            closestCrossLocal = crossLocal;
                            //frameAccess = storedAccess[frameNr]; //

                        }

                    }
                }

                /*
                Vector3f intersectionPointWorld = getIntersectionOfPlane(volumePoint, distance, imagePlaneNormal);
                Vector3f intersectionPointLocal = getLocalIntersectionOfPlane(intersectionPointWorld, thisFrameInverseTransform);
                if (isWithinFrame(intersectionPointLocal, thisFrameSize, 0.5f, 0.5f)){
                float p = getPixelValue(intersectionPointLocal);
                float w = 1 - (distance / df); //Or gaussian for trail
                accumulateValuesInVolume(volumePoint, p, w);
                }

                // Project into plane
                Vector3f planeNormal = framePlaneNormalList[closestFrameNr];
                AffineTransformation::pointer frameInvTrans = frameInverseTransformList[closestFrameNr];
                Image::pointer frame = frameList[closestFrameNr];
                Vector3f crossWorld = getIntersectionOfPlane(pos, closestDist, planeNormal);
                Vector3f crossLocal = getLocalIntersectionOfPlane(crossWorld, frameInvTrans);
                if (isWithinFrame(crossLocal, frame->getSize(), 0.5f, 0.5f)){
                // Get pixel value
                float p = getPixelValue(intersectionPointLocal);
                }
                else {

                }
                */

                if (closestFrameNr != -1){
                    //std::cout << "Closest frameNr: " << closestFrameNr << " to pos " << pos.x() << "-" << pos.y() << "-" << pos.z() << " dist " << closestDist << std::endl;
                    Image::pointer frame = frameList[closestFrameNr];
                    frameAccess = frame->getImageAccess(ACCESS_READ);
                    p = getPixelValue(closestCrossLocal);
                    frameAccess->release();
                }

                // Store value
                volAccess->setScalar(pos, p, 0);
                //unsigned int loc = x + y*size.x() + z*size.x()*size.y(); // )* nrOfComponents
                //outputData[loc] = p;

            }

        }
        std::cout << "!" << std::endl;
    }
    volAccess->release();
    std::cout << "\nDONE calculations!" << std::endl;
}

void Us3Dhybrid::recompileAlgorithmOpenCLCode(){
    //CHECK if dv, rmax or volumeSize changed..
    if (AccumulationVolume->getDimensions() == mDimensionCLCodeCompiledFor &&
        AccumulationVolume->getDataType() == mTypeCLCodeCompiledFor &&
        dv == mDvCompiledFor &&
        Rmax == mRmaxCompiledFor &&
        volumeSize == mVolumeSizeCompiledFor)
        return;

    std::cout << "Recompiling Algorithm OpenCL code";
    OpenCLDevice::pointer device = getMainDevice();
    std::string buildOptions = "";

    if (!device->isWritingTo3DTexturesSupported()) {
        switch (mOutputType) {
        case TYPE_FLOAT:
            buildOptions += " -DTYPE=float";
            break;
        case TYPE_INT8:
            buildOptions += " -DTYPE=char";
            break;
        case TYPE_UINT8:
            buildOptions += " -DTYPE=uchar";
            break;
        case TYPE_INT16:
            buildOptions += " -DTYPE=short";
            break;
        case TYPE_UINT16:
            buildOptions += " -DTYPE=ushort";
            break;
        }
    }
    /*
    int maskSize = int(mMaskSize);
    buildOptions += " -D FILTER_SIZE=";
    buildOptions += std::to_string(maskSize);
    std::cout << "maskSize " << maskSize << std::endl;
    */
    buildOptions += " -D DV=";
    buildOptions += std::to_string(dv);
    std::cout << " -D DV=" << dv << std::endl;

    buildOptions += " -D R_MAX=";
    buildOptions += std::to_string(Rmax);
    std::cout << " -D R_MAX=" << Rmax << std::endl;

    buildOptions += " -D VOL_SIZE_X=";
    buildOptions += std::to_string(volumeSize(0));
    std::cout << " -D VOL_SIZE_X=" << volumeSize(0) << std::endl;
    buildOptions += " -D VOL_SIZE_Y=";
    buildOptions += std::to_string(volumeSize(1));
    std::cout << " -D VOL_SIZE_Y=" << volumeSize(1) << std::endl;
    buildOptions += " -D VOL_SIZE_Z=";
    buildOptions += std::to_string(volumeSize(2));
    std::cout << " -D VOL_SIZE_Z=" << volumeSize(2) << std::endl;

    float bufferXY = 0.5f;
    float bufferZ = 0.5f;
    buildOptions += " -D BUFFER_XY=";
    buildOptions += std::to_string(bufferXY);
    std::cout << " -D BUFFER_XY=" << bufferXY << std::endl;
    buildOptions += " -D BUFFER_Z=";
    buildOptions += std::to_string(bufferZ);
    std::cout << " -D BUFFER_Z=" << bufferZ << std::endl;

    cl::Program programUs3Dhybrid = getOpenCLProgram(device, "us3Dhybrid", buildOptions);
    mKernel = cl::Kernel(programUs3Dhybrid, "accumulateFrameToVolume");

    mDimensionCLCodeCompiledFor = AccumulationVolume->getDimensions();
    mTypeCLCodeCompiledFor = AccumulationVolume->getDataType();
    mDvCompiledFor = dv;
    mRmaxCompiledFor = Rmax;
    mVolumeSizeCompiledFor = volumeSize; //or x,y,z?
}

void Us3Dhybrid::executeOpenCLTest(){
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/UsReconstruction/testReadWrite.cl", "testReadWrite");

    OpenCLDevice::pointer clDevice = getMainDevice();
    std::string buildOptions = "";
    buildOptions += " -DTYPE=float";
    /*
    if (!device->isWritingTo3DTexturesSupported()) {
        switch (mOutputType) {
        case TYPE_FLOAT:
            buildOptions += " -DTYPE=float";
            break;
        case TYPE_INT8:
            buildOptions += " -DTYPE=char";
            break;
        case TYPE_UINT8:
            buildOptions += " -DTYPE=uchar";
            break;
        case TYPE_INT16:
            buildOptions += " -DTYPE=short";
            break;
        case TYPE_UINT16:
            buildOptions += " -DTYPE=ushort";
            break;
        }
    }
    */
    Vector3i size = Vector3i(3, 3, 3);
    buildOptions += " -D VOL_SIZE_X=";
    buildOptions += std::to_string(size(0));
    std::cout << " -D VOL_SIZE_X=" << size(0) << std::endl;
    buildOptions += " -D VOL_SIZE_Y=";
    buildOptions += std::to_string(size(1));
    std::cout << " -D VOL_SIZE_Y=" << size(1) << std::endl;
    buildOptions += " -D VOL_SIZE_Z=";
    buildOptions += std::to_string(size(2));
    std::cout << " -D VOL_SIZE_Z=" << size(2) << std::endl;
    cl::Program programTest = getOpenCLProgram(clDevice, "testReadWrite", buildOptions);
    cl::Kernel testKernel = cl::Kernel(programTest, "addToVolume");

    int bufferSize = size(0)*size(1)*size(2);
    int components = 2;
    float * volumeData = new float[bufferSize*components];
    float * semaphorData = new float[bufferSize];

    mCLVolume = cl::Buffer(
        clDevice->getContext(),
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(float)*bufferSize*components,
        volumeData
    ); //CL_MEM_READ_ONLY //CL_MEM_COPY_HOST_PTR, //CL_MEM_ALLOC_HOST_PTR

    cl::Buffer mCLSemaphore = cl::Buffer(
        clDevice->getContext(),
        CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        sizeof(int)*bufferSize
    );
    cl::CommandQueue cmdQueue = clDevice->getCommandQueue();
    cl::NDRange globalSize = cl::NDRange(size(0), size(1), size(2));
    float toAdd = 100.0f;
    testKernel.setArg(0, mCLVolume);
    testKernel.setArg(1, mCLSemaphore);
    testKernel.setArg(2, toAdd);
    cmdQueue.enqueueNDRangeKernel(
        testKernel,
        cl::NullRange,
        globalSize,
        cl::NullRange
        );
    cmdQueue.finish();

    for (int x = 0; x < size.x(); x++){
        for (int y = 0; y < size.y(); y++){
            for (int z = 0; z < size.z(); z++){
                int loc = x + y*size.x() + z*size.x()*size.y();
                float p = volumeData[loc * 2];
                float w = volumeData[loc * 2 + 1];
                //int s = semaphorData[loc];
                float pw = p*w;
            }
        }
    }
}

void Us3Dhybrid::executeAlgorithm(){
    ExecutionDevice::pointer device = getMainDevice();
    if (!runCLhybrid){//device->isHost()) {
        std::cout << "Executing on host" << std::endl;
        executeAlgorithmOnHost(); // Run on CPU instead
        return;
    }
    if (false){
        executeOpenCLTest();
    }

    OpenCLDevice::pointer clDevice = device;
    /*
    cl::Context clContext = clDevice->getContext();
    cl::Platform clPlatform = clDevice->getPlatform();
    cl_int err;
    clPlatform.getInfo(&err);
    //clDevice->isImageFormatSupported(??)
    */
    // TODO Fix a kernel and so on
    setOutputType(AccumulationVolume->getDataType());
    recompileAlgorithmOpenCLCode();
    std::cout << "Alg CL part 1" << std::endl;
    //OpenCLImageAccess::pointer clVolAccess = AccumulationVolume->getOpenCLImageAccess(ACCESS_READ_WRITE, clDevice);
    if (!volAccess)
        volAccess = AccumulationVolume->getImageAccess(ACCESS_READ_WRITE);
    float * volumeData = (float*)volAccess->get();
    //cl::Buffer 
    //mCLVolume;
    std::cout << "Alg CL part 2" << std::endl;
    int bufferSize = volumeSize(0)*volumeSize(1)*volumeSize(2);
    int components = 2;
    mCLVolume = cl::Buffer(
        clDevice->getContext(),
        CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        sizeof(float)*bufferSize*components,
        volumeData
    ); //CL_MEM_READ_ONLY //CL_MEM_COPY_HOST_PTR,

    cl::Buffer mCLSemaphore = cl::Buffer(
        clDevice->getContext(),
        CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        sizeof(int)*bufferSize
    );
    std::cout << "Alg CL part 3" << std::endl;
    cl::CommandQueue cmdQueue = clDevice->getCommandQueue();

    for (int frameNr = 0; frameNr < frameList.size(); frameNr++){
        // Get FRAME
        std::cout << "Running for frame #" << frameNr << std::endl;
        Image::pointer frame = frameList[frameNr];
        // Calc imagePlaneNormal and dominating direction of it
        Vector3f imagePlaneNormal = framePlaneNormalList[frameNr]; //getImagePlaneNormal(frame);
        int domDir = getDominatingVectorDirection(imagePlaneNormal);
        float domVal = fabs(imagePlaneNormal(domDir));
        // Get current, last and next plane
        Vector3f thisFrameRootPoint = frameBaseCornerList[frameNr];
        Vector3ui thisFrameSize = frame->getSize();
        float thisFramePlaneDvalue = framePlaneDValueList[frameNr];
        AffineTransformation::pointer thisFrameInverseTransform = frameInverseTransformList[frameNr];
        Vector3f lastFrameRootPoint, lastFrameNormal, nextFrameRootPoint, nextFrameNormal;
        if (frameNr != 0){
            lastFrameRootPoint = frameBaseCornerList[frameNr - 1];
            lastFrameNormal = framePlaneNormalList[frameNr - 1];
        }
        if (frameNr != frameList.size() - 1){
            nextFrameRootPoint = frameBaseCornerList[frameNr + 1];
            nextFrameNormal = framePlaneNormalList[frameNr + 1];
        }
        // Get frame access
        OpenCLImageAccess::pointer clFrameAccess = frame->getOpenCLImageAccess(ACCESS_READ, clDevice);

        // Find size of non-dominating directions in volume space (a-dir & b-dir)
        Vector2i aDirRange = getFrameRangeInVolume(frameNr, domDir, 0); //a: 0
        Vector2i bDirRange = getFrameRangeInVolume(frameNr, domDir, 1); //b: 1
        int aDirStart = aDirRange(0);
        int bDirStart = bDirRange(0);
        int aDirSize = aDirRange(1) - aDirRange(0); //mod by -1 or +1 elns?
        int bDirSize = bDirRange(1) - bDirRange(0); //mod by -1 or +1 elns?
        //For each a in a-dir
        //for (int a = aDirRange(0); a <= aDirRange(1); a++){
        //For each b in b-dir
        //for (int b = bDirRange(0); b <= bDirRange(1); b++){
        
        // Define OpenCL variables
        cl_int2 startOffset = { aDirStart, bDirStart };
        cl_int3 volSize = { volumeSize(0), volumeSize(1), volumeSize(2) };
        cl_float3 imgNormal = { imagePlaneNormal(0), imagePlaneNormal(1), imagePlaneNormal(2) };
        cl_float3 imgRoot = { thisFrameRootPoint(0), thisFrameRootPoint(1), thisFrameRootPoint(2) };
        cl_int2 imgSize = { thisFrameSize(0), thisFrameSize(1) };// int2 imgSize,
        cl_float3 lastNormal = { lastFrameNormal(0), lastFrameNormal(1), lastFrameNormal(2) };
        cl_float3 lastRoot = { lastFrameRootPoint(0), lastFrameRootPoint(1), lastFrameRootPoint(2) };
        cl_float3 nextNormal = { nextFrameNormal(0), nextFrameNormal(1), nextFrameNormal(2) };
        cl_float3 nextRoot = { nextFrameRootPoint(0), nextFrameRootPoint(1), nextFrameRootPoint(2) };
        //store inverseAffineTransformation?
        //thisFrameInverseTransform->matrix()
        cl_float16 imgInvTrans = transform4x4tofloat16(thisFrameInverseTransform);
        cl_int outputDataType = AccumulationVolume->getDataType();
        //cl::Buffer mCLMask; ??
        /*mCLMask = cl::Buffer(
        clDevice->getContext(),
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(float)*bufferSize,
        mMask
        );
        */
        cl::NDRange globalSize = cl::NDRange(aDirSize, bDirSize);
        cl::NDRange workOffset = cl::NDRange(aDirStart, bDirStart); // ev. cl::NullRange

        mKernel.setArg(0, *clFrameAccess->get2DImage());
        mKernel.setArg(1, mCLVolume);// *clVolAccess->get3DImage());
        //mKernel.setArg(2, dv); //CAN be defined in buildString
        //mKernel.setArg(3, Rmax); //CAN be defined in buildString
        mKernel.setArg(2, domDir);
        mKernel.setArg(3, domVal);
        //mKernel.setArg(6, volSize);// volumeSize); //Vector3i? to int3? //CAN be defined in buildString
        mKernel.setArg(4, imgNormal);// imagePlaneNormal); //Vector3f
        mKernel.setArg(5, imgRoot);// thisFrameRootPoint); //Vector3f
        mKernel.setArg(6, thisFramePlaneDvalue);
        mKernel.setArg(7, imgSize);// thisFrameSize); //Vector3i //CAN be defined in buildString MAYBE if all same size
        mKernel.setArg(8, lastNormal);// lastFrameNormal); //Vector3f
        mKernel.setArg(9, lastRoot);// lastFrameRootPoint); //Vector3f
        mKernel.setArg(10, nextNormal);// nextFrameNormal); //Vector3f
        mKernel.setArg(11, nextRoot);// nextFrameRootPoint); //Vector3f
        mKernel.setArg(12, imgInvTrans);// thisFrameInverseTransform->matrix()); // AffineTransformation::pointer? store as something else?
        mKernel.setArg(13, startOffset);
        mKernel.setArg(14, outputDataType); // should be int like CLK_FLOAT
        //CAN define bufferXY or bufferZ in buildString
        mKernel.setArg(15, mCLSemaphore);

        cmdQueue.enqueueNDRangeKernel(
            mKernel,
            workOffset,
            globalSize,
            cl::NullRange
            );
        cmdQueue.finish();
        //cmdQueue.finish();
        //clFinish(cmdQueue);
    }
    //mCLSemaphore
    //clFinish(cmdQueue);
    //cmdQueue.finish();

}

// CPU algoritme
//template <class T>
void Us3Dhybrid::executeAlgorithmOnHost(){
    //Get access to volume on which we accumulate the values in
    // (volAccess is defined globally in Us3Dhybrid as an ImageAccess::pointer)
    volAccess = AccumulationVolume->getImageAccess(accessType::ACCESS_READ_WRITE);
   // float * outputData 
    volData= (float*)volAccess->get();
    yLocMultiplier = volumeSize.x();
    zLocMultiplier = volumeSize.x() * volumeSize.y();
    
    std::cout << "Running with dv: " << dv << " and Rmax: " << Rmax << std::endl;

    Vector3ui frameSizeWrites = firstFrame->getSize(); //Vector3ui
    uint * writesToPixel = new uint[frameSizeWrites.x()*frameSizeWrites.y()];
    {
        for (int x = 0; x < frameSizeWrites.x(); x++){
            for (int y = 0; y < frameSizeWrites.y(); y++){
                int loc = x + y*frameSizeWrites.x();
                writesToPixel[loc] = 0;
            }
        }
    }
    int itTotal = 0;
    int it2Total = 0;
    clock_t startLoopTime = clock();
    float dfSum; int it; int it2; clock_t startFrameTime; //LOCAL per frame
    // For each FRAME
    for (int frameNr = 0; frameNr < frameList.size(); frameNr++){
        // Get FRAME
        std::cout << "----------------------- Running frame #" << frameNr << " of tot " << frameList.size() << "--------------" << std::endl;
        Image::pointer frame = frameList[frameNr];
        //PNN option
        if (runAsPNNonly){
            executeFramePNN(frame);
            continue;
        }
        // Calc imagePlaneNormal and dominating direction of it
        Vector3f imagePlaneNormal = framePlaneNormalList[frameNr]; //getImagePlaneNormal(frame);
        int domDir = getDominatingVectorDirection(imagePlaneNormal);
        float domVal = fabs(imagePlaneNormal(domDir));
        // Get current, last and next plane
        Vector3f thisFrameRootPoint = frameBaseCornerList[frameNr];
        Vector3ui thisFrameSize = frame->getSize();
        float thisFramePlaneDvalue = framePlaneDValueList[frameNr];
        AffineTransformation::pointer thisFrameInverseTransform = frameInverseTransformList[frameNr];
        {
            Vector3f pointVolZero = thisFrameInverseTransform->multiply(Vector3f(0, 0, 0));
            Vector3f volMax = Vector3f(volumeSize(0) - 1.0f, volumeSize(1) - 1.0f, volumeSize(2) - 1.0f);
            Vector3f pointVolHero = thisFrameInverseTransform->multiply(volMax);
            Matrix4f invMatrix = thisFrameInverseTransform->matrix();
            Vector4f t0 = invMatrix.row(0);
            Vector4f t1 = invMatrix.row(1);
            Vector4f t2 = invMatrix.row(2);
            Vector4f t3 = invMatrix.row(3);
            bool tFoo = false;
        }
        Vector3f lastFrameRootPoint, lastFrameNormal, nextFrameRootPoint, nextFrameNormal;
        {
            if (frameNr != 0){
                lastFrameRootPoint = frameBaseCornerList[frameNr - 1];
                lastFrameNormal = framePlaneNormalList[frameNr - 1];
            }
            if (frameNr != frameList.size() - 1){
                nextFrameRootPoint = frameBaseCornerList[frameNr + 1];
                nextFrameNormal = framePlaneNormalList[frameNr + 1];
            }
        }
        {
            // STATISTICS 
            dfSum = 0.0f;
            it = 0;
            it2 = 0;
            //cpu time clock()
            //wall time time()
            startFrameTime = clock();
            // END STATS
        }
        
        frameAccess = frame->getImageAccess(accessType::ACCESS_READ); // Get frame access // ImageAccess::pointer global
        //TODOOOO
        //T * inputData = (T*)inputAccess->get();
        //T * outputData = (T*)outputAccess->get();
        frameData = (int*)frameAccess->get();
        frameSize = frame->getSize();
        frameChannels = frame->getNrOfComponents();
        frameType = frame->getDataType();

        // Find size of non-dominating directions in volume space (a-dir & b-dir)
        Vector2i aDirRange = getFrameRangeInVolume(frameNr, domDir, 0); //a: 0
        Vector2i bDirRange = getFrameRangeInVolume(frameNr, domDir, 1); //b: 1
        for (int a = aDirRange(0); a <= aDirRange(1); a++){ //For each a in a-dir
            //std::cout << ".";
            for (int b = bDirRange(0); b <= bDirRange(1); b++){ //For each b in b-dir
                //Find basePoint in the plane based on the a and b values
                Vector3f basePoint = getBasePointInPlane(thisFrameRootPoint, imagePlaneNormal, thisFramePlaneDvalue, a, b, domDir);
                //TODO determine if reasonably close to plane? Elimination/speedup (use inverseTrans)
                /*Vector3f crossLocal = getLocalIntersectionOfPlane(basePoint, thisFrameInverseTransform);
                if (!isWithinFrame(crossLocal, thisFrameSize, 1.0f, 0.5f)){
                    continue;
                }*/

                //Find distance to last and next frame
                float d1 = getDistanceAlongNormal(basePoint, imagePlaneNormal, lastFrameRootPoint, lastFrameNormal); //TODO check if correct
                float d2 = getDistanceAlongNormal(basePoint, imagePlaneNormal, nextFrameRootPoint, nextFrameNormal);
                //Calculate half width df and dfDom
                float df = calculateHalfWidth(d1, d2, dv, Rmax);
                float dfDom = df / domVal;
                {
                    dfSum += df;
                    it++;
                }
                //Indeks for c-dir range in domDir
                Vector2i cDirRange = getDomDirRange(basePoint, domDir, dfDom, volumeSize);
                for (int c = cDirRange(0); c <= cDirRange(1); c++){ //For hver c i c-dir
                    Vector3i volumePoint = getVolumePointLocation(a, b, c, domDir);
                    if (volumePointOutsideVolume(volumePoint, volumeSize)){
                        continue;
                    } it2++;
                    float distance = getPointDistanceAlongNormal(volumePoint, thisFrameRootPoint, imagePlaneNormal);
                    if (fabs(distance) >= df){ continue; }
                    Vector3f intersectionPointWorld = getIntersectionOfPlane(volumePoint, distance, imagePlaneNormal);
                    Vector3f intersectionPointLocal = getLocalIntersectionOfPlane(intersectionPointWorld, thisFrameInverseTransform);
                    if (isWithinFrame(intersectionPointLocal, thisFrameSize, 0.5f, 0.5f)){
                        //float p = getPixelValue(intersectionPointLocal);
                        float p = getPixelValueData(intersectionPointLocal);
                        //float absDist = fabs(distance);
                        float w = 1 - (fabs(distance) / df); //Or gaussian for trail
                        //accumulateValuesInVolume(volumePoint, p, w);
                        accumulateValuesInVolumeData(volumePoint, p, w);
                        {
                            //Stats
                            int loc = round(intersectionPointLocal.x()) + round(intersectionPointLocal.y())*frameSize.x();
                            writesToPixel[loc]++;
                        }
                    }
                }
            }
        }
        frameAccess->release();
        //std::cout << "!" << std::endl;

        // FINAL STATS
        float dfAvg = dfSum / it; 
        {
            std::cout << "Avg DF: " << dfAvg << " and tot iter.: " << it << " sub: " << it2 << std::endl;
            itTotal += it;
            it2Total += it2;
            clock_t endFrameTime = clock();
            clock_t clockTicksTaken = endFrameTime - startFrameTime;
            double timeInSeconds = clockTicksTaken / (double)CLOCKS_PER_SEC;
            clock_t clockTicksTakenLoop = endFrameTime - startLoopTime;
            double timeInSecondsLoop = clockTicksTakenLoop / (double)CLOCKS_PER_SEC;
            int minutesInLoop = timeInSecondsLoop / 60;
            int secondsInMinute = ((int)timeInSecondsLoop) % 60;
            int percentComplete = ((frameNr + 1) * 100) / (frameList.size());
            int estimateTotalTimeSec = (timeInSecondsLoop / (double)(frameNr + 1)) * frameList.size();
            int estTotMin = floor(estimateTotalTimeSec / 60);
            int estTotSec = estimateTotalTimeSec % 60;
            int estimateSecRemain = (1 - ((double)(frameNr + 1) / (double)frameList.size())) * estimateTotalTimeSec;
            int estRemMin = floor(estimateSecRemain / 60);
            int estRemSec = estimateSecRemain % 60;
            std::cout << "Tick: " << clockTicksTaken << " & " << timeInSeconds << "s! " << percentComplete << "% in " << minutesInLoop << "m" << secondsInMinute << "s" << " | Est. tot: " << estTotMin << "m" << estTotSec << "s; Rem.: " << estRemMin << "m" << estRemSec << "s!" << std::endl;
            //<< estimateTotalTimeSec << "s; Remains: " << estimateSecRemain << "s!" << std::endl; //<< "Spent "// << timeInSecondsLoop << "s or "
            //std::cout << "Est. tot: " << estimateTotalTimeSec << "s; Remains: " << estimateSecRemain << "s!" << std::endl;
            std::cout << "" << std::endl;
        }
    }
    clock_t endLoopTime = clock();
    {
        std::cout << " ####################################################### " << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Finished running with total iterations: " << itTotal << " and subloops: " << it2Total << "!" << std::endl;

        clock_t clockTicksTakenLoop = endLoopTime - startLoopTime;
        double timeInSecondsLoop = clockTicksTakenLoop / (double)CLOCKS_PER_SEC;
        int minutesInLoop = timeInSecondsLoop / 60;
        int secondsInMinute = ((int)timeInSecondsLoop) % 60;
        std::cout << "Algorithm spent in total " << clockTicksTakenLoop << " clock ticks and " << timeInSecondsLoop << " seconds!" << std::endl;
        std::cout << "That is " << minutesInLoop << "minutes and " << secondsInMinute << "seconds" << std::endl;
        std::cout << "Spent " << (timeInSecondsLoop / itTotal) << "sec per iteration and " << (timeInSecondsLoop / it2Total) << "sec per subloop!" << std::endl;

        //Frame read stats
        /*#include "FAST/Visualization/SimpleWindow.hpp"
        #include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
        #include "FAST/TestDataPath.hpp"*/
        //#include "FAST/Importers/ImageImporter.hpp"

        Image::pointer readStatsImg = Image::New();
        //readStatsImg->createFromImage(firstFrame);
        readStatsImg->create(frameSize, DataType::TYPE_UINT16, 1);
        ImageAccess::pointer readImgAcc = readStatsImg->getImageAccess(ACCESS_READ_WRITE);
        for (int x = 0; x < frameSize.x(); x++){
            for (int y = 0; y < frameSize.y(); y++){
                int loc = x + y*frameSize.x();
                Vector3i pos = Vector3i(x, y, 0);
                uint value = writesToPixel[loc];
                readImgAcc->setScalar(pos, value, 0);
            }
        }
        readImgAcc->release();
        //SimpleWindow::pointer window = SimpleWindow::New();
        //window->addRenderer(sRenderer);
        //window->start();
        std::string _filePath = std::string(FAST_TEST_DATA_DIR) + "/output/";
        std::string output_filename = _filePath + "HybridFrameReadImg.jpg";
        std::cout << "Output filename: " << output_filename << std::endl;

        ImageExporter::pointer exporter = ImageExporter::New();
        exporter->setFilename(output_filename);
        exporter->setInputData(readStatsImg);
        exporter->update();
    }
    
    //volAccess->release();
}

void Us3Dhybrid::generateOutputVolume(){
    std::cout << "Final reconstruction calculations!" << std::endl;
    // Finally, calculate reconstructed volume
    setOutputType(AccumulationVolume->getDataType());
    std::cout << "Step 1" << std::endl;
    outputVolume = getStaticOutputData<Image>(0);
    std::cout << "Step 2" << std::endl;
    outputVolume->create(AccumulationVolume->getSize(), AccumulationVolume->getDataType(), 1); //1-channeled outputVolume
    std::cout << "Step 3" << std::endl;
    //outputVolume->setSpacing(Vector3f(1.0f, 1.0f, 1.0f));
    Vector3f frameSpacing = firstFrame->getSpacing();
    float spacingVal = mVoxelSpacing * globalScalingValue; //dv * globalScalingValue;
    Vector3f outputSpacing = Vector3f(spacingVal, spacingVal, spacingVal);// 0.1f, 0.1f, 0.1f);
    outputVolume->setSpacing(outputSpacing);

    /*
    ExecutionDevice::pointer device = getMainDevice();
    if (!device->isHost()) {
        generateOutputVolume(device); // Run on GPU instead
        return;
    }
    */
    std::cout << "Step 4" << std::endl;
    if (!volAccess)
        volAccess = AccumulationVolume->getImageAccess(ACCESS_READ_WRITE);
    std::cout << "Step 5" << std::endl;
    ImageAccess::pointer outAccess = outputVolume->getImageAccess(ACCESS_READ_WRITE);
    std::cout << "Step 6" << std::endl;
    //T * inputData = (T*)inputAccess->get();
    //T * outputData = (T*)outputAccess->get();
    float * inputData = (float*)volAccess->get();
    float * outputData = (float*)outAccess->get();
    std::cout << "Step 7" << std::endl;
    uint width = outputVolume->getWidth();
    uint height = outputVolume->getHeight();
    uint depth = outputVolume->getDepth();
    uint nrOfComponents = AccumulationVolume->getNrOfComponents();
    for (uint x = 0; x < width; x++){
        std::cout << ".";
        for (uint y = 0; y < height; y++){
            for (uint z = 0; z < depth; z++){
                unsigned int loc = x + y*width + z*width*height;
                float p = inputData[loc*nrOfComponents];
                float w = inputData[loc*nrOfComponents + 1];
                if (w > 0.0 && p >= 0.0){ // w != 0.0 to avoid division error // This other logic to avoid uninitialized voxels
                    float finalP = p / w;
                    outputData[loc] = finalP;
                }
                else{
                    outputData[loc] = 0.0f;
                }
                /*
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
                */
            }
        }
    }
    outAccess->release();
    std::cout << "\nDONE calculations!" << std::endl;
    //Can possibly make 2D slices here or alternatively to the one above
}

void Us3Dhybrid::recompileNormalizeOpenCLCode(){
    if (AccumulationVolume->getDimensions() == mDimensionCLCodeCompiledFor &&
        AccumulationVolume->getDataType() == mTypeCLCodeCompiledFor)
        return;

    OpenCLDevice::pointer device = getMainDevice();
    std::string buildOptions = "";
    
    if (!device->isWritingTo3DTexturesSupported()) {
        switch (mOutputType) {
        case TYPE_FLOAT:
            buildOptions += " -DTYPE=float";
            break;
        case TYPE_INT8:
            buildOptions += " -DTYPE=char";
            break;
        case TYPE_UINT8:
            buildOptions += " -DTYPE=uchar";
            break;
        case TYPE_INT16:
            buildOptions += " -DTYPE=short";
            break;
        case TYPE_UINT16:
            buildOptions += " -DTYPE=ushort";
            break;
        }
    }
    
    cl::Program programNormalize = getOpenCLProgram(device, "normalizeVolume", buildOptions);
    mKernelNormalize = cl::Kernel(programNormalize, "normalizeVolume");

    mDimensionCLCodeCompiledFor = outputVolume->getDimensions();
    mTypeCLCodeCompiledFor = outputVolume->getDataType();
}

void Us3Dhybrid::generateOutputVolume(ExecutionDevice::pointer device){
    std::cout << "Running on GPU " << device->getStaticNameOfClass() << std::endl;

    OpenCLDevice::pointer clDevice = device;
    //recompileOpenCLCode(input)
    recompileNormalizeOpenCLCode();
    OpenCLImageAccess::pointer inputAccess = AccumulationVolume->getOpenCLImageAccess(ACCESS_READ, device);
    cl::NDRange globalSize = cl::NDRange(volumeSize(0), volumeSize(1), volumeSize(2));
    OpenCLImageAccess::pointer outputAccess = outputVolume->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    std::cout << "Access fetched" << std::endl;
    mKernelNormalize.setArg(0, *inputAccess->get3DImage());
    mKernelNormalize.setArg(1, *outputAccess->get3DImage());

    cl::CommandQueue cmdQueue = clDevice->getCommandQueue();
    cmdQueue.enqueueNDRangeKernel(
        mKernelNormalize,
        cl::NullRange,
        globalSize,
        cl::NullRange
        );
    std::cout << "Added to cmdQueue" << std::endl;
    std::cout << "DONE calculations!" << std::endl;
    inputAccess->release();
    outputAccess->release();
}

void Us3Dhybrid::initVolume(Image::pointer rootFrame){
    //Using object-defined variables firstFrame, frameList and possible preset variables
    //Find initial transform so that firstFrame corner(0,0) is voxel(0,0,0) in volume 
    //and all pixels in this frame would be in the z=0 plane
     //inverseTransform();

    Image::pointer testFrame = frameList[0];// 10]; //TO TEST THIS

    //INVERSE TRANSFORM
    AffineTransformation::pointer inverseSystemTransform = getInverseTransformation(rootFrame);
    //std::vector<AffineTransformation::pointer> frameTransformList = {}; // std::vector global AffineTransformation::pointer
    Vector3f minCoords; {
        addTransformationToFrame(rootFrame, inverseSystemTransform);
        //AffineTransformation::pointer oldImgTransform = SceneGraph::getAffineTransformationFromData(rootFrame);
        
        //Transform all frames according to initial transform
        // & Find minimum

        BoundingBox box1 = rootFrame->getTransformedBoundingBox();
        Vector3f corner = box1.getCorners().row(0);
        minCoords(0) = corner(0);
        minCoords(1) = corner(1);
        minCoords(2) = corner(2);
        for (int i = 0; i < frameList.size(); i++){
            Image::pointer frame = frameList[i];
            // Start transforming frame
            if (i != 0)
                addTransformationToFrame(frame, inverseSystemTransform);

            /*
            if (i != 0){
                AffineTransformation::pointer oldImgTransform = SceneGraph::getAffineTransformationFromData(rootFrame);
            }
            
            AffineTransformation::pointer newImgTransform = inverseSystemTransform->multiply(oldImgTransform);
            // TODO AT LAST Round instead: frame->getSceneGraphNode()->setTransformation(newImgTransform);
            frameTransformList.push_back(newImgTransform);
            */

            //AffineTransformation::pointer oldImgTransform = SceneGraph::getAffineTransformationFromData(frame);
            //AffineTransformation::pointer newImgTransform = oldImgTransform->multiply(inverseSystemTransform);
            //frame->getSceneGraphNode()->setTransformation(newImgTransform);
            // Check corners for minimum
            BoundingBox box = frame->getTransformedBoundingBox();
            MatrixXf corners = box.getCorners();
            for (int j = 0; j < 8; j++){
                for (int k = 0; k < 3; k++){
                    float pointValue = corners(j, k);
                    if (pointValue < minCoords(k))
                        minCoords(k) = pointValue;
                }
            }
        }
    }
    
    //Test frame 10
    AffineTransformation::pointer testTransformation = SceneGraph::getAffineTransformationFromData(testFrame);
    Vector3f pointZero = testTransformation->multiply(Vector3f(0, 0, 0));
    Vector3f pointHero = testTransformation->multiply(Vector3f(350, 546, 0));
    Matrix4f testMatrix = testTransformation->matrix();
    Vector4f t0 = testMatrix.row(0);
    Vector4f t1 = testMatrix.row(1);
    Vector4f t2 = testMatrix.row(2);
    Vector4f t3 = testMatrix.row(3);
    bool tFoo = false;

    /**Transform all frames so that minimum corner is (0,0,0) //Just translate right?
    * & Find min/max in each coordinate direction x/y/z
    * & Store min/max/base/normal for each frame
    */
    //MINIMUM TRANSFORM + scale to fit
    AffineTransformation::pointer transformToMinimum = getTranslationFromMinCoordsVector(minCoords); //TODO extract these to methods
    AffineTransformation::pointer totalTransform;
    Matrix4f totalMatrix;
    Vector3f maxCoords; {
        addTransformationToFrame(rootFrame, transformToMinimum);
        //Test frame 10
        AffineTransformation::pointer testTransformationMin = SceneGraph::getAffineTransformationFromData(testFrame);
        pointZero = testTransformationMin->multiply(Vector3f(0, 0, 0));
        pointHero = testTransformationMin->multiply(Vector3f(350, 546, 0));
        Matrix4f testMatrixMin = testTransformationMin->matrix();
        t0 = testMatrix.row(0);
        t1 = testMatrix.row(1);
        t2 = testMatrix.row(2);
        t3 = testMatrix.row(3);
        tFoo = false;

        BoundingBox box2 = rootFrame->getTransformedBoundingBox();

        //Scale to keep pixel/voxel spacing basicall nothing now - after fix! but can add scaling here!
        {
            BoundingBox boxUntransformed = rootFrame->getBoundingBox();
            MatrixXf cTrans = box2.getCorners();
            MatrixXf cUtrans = boxUntransformed.getCorners();
            Vector3f i0 = cUtrans.row(0);
            Vector3f i1 = cUtrans.row(1);
            Vector3f i2 = cUtrans.row(2);
            Vector3f i3 = cUtrans.row(3);
            i0 = cTrans.row(0);
            i1 = cTrans.row(1);
            i2 = cTrans.row(2);
            i3 = cTrans.row(3);
            int boxSize = cUtrans.size();
            Vector3f tZeroCorner = cTrans.row(0);
            Vector3f tEndCorner = cTrans.row(2); 
            Vector3f tDist = tEndCorner - tZeroCorner;
            Vector3f uZeroCorner = cUtrans.row(0);
            Vector3f uEndCorner = cUtrans.row(2); 
            Vector3f uDist = uEndCorner - uZeroCorner;
            float scaleX = uDist.x() / tDist.x();
            float scaleY = uDist.y() / tDist.y();
            //Ignoring Z
            Vector3f scaling = Vector3f(scaleX, scaleY, 1);
            AffineTransformation::pointer scaleOneTransform = getScalingFromVector(scaling);
            addTransformationToFrame(rootFrame, scaleOneTransform);
            //Testing
            box2 = rootFrame->getTransformedBoundingBox();
            Vector3f cornerA = box2.getCorners().row(0);
            Vector3f cornerB = box2.getCorners().row(2);
            // ADD scaleTransform to transformToMinimum
            transformToMinimum = scaleOneTransform->multiply(transformToMinimum);
            /*Matrix4f minMatrix = transformToMinimum->matrix();
            Vector4f t0 = minMatrix.row(0);
            Vector4f t1 = minMatrix.row(1);
            Vector4f t2 = minMatrix.row(2);
            Vector4f t3 = minMatrix.row(3);
            bool foo2 = false;*/
            //DONE scaling

        }
        
        box2 = rootFrame->getTransformedBoundingBox();
        Vector3f corner2 = box2.getCorners().row(0);

        minCoords(0) = corner2(0); //or 0
        minCoords(1) = corner2(1);//or 0
        minCoords(2) = corner2(2);//or 0
        maxCoords(0) = corner2(0);
        maxCoords(1) = corner2(1);
        maxCoords(2) = corner2(2);
        for (int i = 0; i < frameList.size(); i++){
            Image::pointer frame = frameList[i];
            // Start transforming frame
            if (i != 0)
                addTransformationToFrame(frame, transformToMinimum);
            // Check corners for minimum
            BoundingBox box = frame->getTransformedBoundingBox();
            MatrixXf corners = box.getCorners();
            for (int j = 0; j < 8; j++){ //8
                for (int k = 0; k < 3; k++){
                    float pointValue = corners(j, k);
                    if (pointValue < minCoords(k))
                        minCoords(k) = pointValue;
                    if (pointValue > maxCoords(k))
                        maxCoords(k) = pointValue;
                }
            }
        }

        //Test total results: inverseSystemTransform & transformToMinimum
        //AffineTransformation::pointer 
        totalTransform = transformToMinimum->multiply(inverseSystemTransform);
        totalMatrix = totalTransform->matrix();
        Vector4f m0 = totalMatrix.row(0);
        Vector4f m1 = totalMatrix.row(1);
        Vector4f m2 = totalMatrix.row(2);
        Vector4f m3 = totalMatrix.row(3);
        Matrix4f minTransformMatrix = transformToMinimum->matrix();
        Vector4f s0 = minTransformMatrix.row(0);
        Vector4f s1 = minTransformMatrix.row(1);
        Vector4f s2 = minTransformMatrix.row(2);
        Vector4f s3 = minTransformMatrix.row(3);
    }
    
    //Test frame 10
    AffineTransformation::pointer testTransformation2 = SceneGraph::getAffineTransformationFromData(testFrame);
    pointZero = testTransformation2->multiply(Vector3f(0, 0, 0));
    pointHero = testTransformation2->multiply(Vector3f(350, 546, 0));
    Matrix4f testMatrix2 = testTransformation2->matrix();
    t0 = testMatrix2.row(0);
    t1 = testMatrix2.row(1);
    t2 = testMatrix2.row(2);
    t3 = testMatrix2.row(3);
    tFoo = false;

    // FIND SCALING
    Vector3f scaling; {
        // Find size current Init volume of size max-min in each direction x/y/z
        Vector3f sizeOne = maxCoords - minCoords;
        // Find scaling
        /*
        float maxSize = 0.f;
        for (int i = 0; i < 3; i++){
            if (sizeOne(i) > maxSize){
                maxSize = sizeOne(i);
            }
        }
        */
        Vector3f spacing = rootFrame->getSpacing();
        if (zDirInitSpacing != 0.0)
            spacing(2) = zDirInitSpacing;
        float wantedSpacing = mVoxelSpacing; //dv
        scaling = Vector3f(0.f, 0.f, 0.f);
        for (int i = 0; i < 3; i++){
            scaling(i) = (spacing(i) / wantedSpacing) * globalScalingValue;
        }
        /*
        Vector3f wantedSize = Vector3f(200.f, 200.f, 200.f); //Can be smaller than 200.f or at least just scale 1 up to 200.f
        float wantedMax = mScaleToMax; //100.f;
        float scalingFactor = wantedMax / maxSize;
        for (int i = 0; i < 3; i++){
        //scaling(i) = wantedSize(i) / sizeOne(i);
        scaling(i) = scalingFactor;
        }
        */
    }
    
    // SCALING TRANSFORM
    AffineTransformation::pointer scaleTransform = getScalingFromVector(scaling);
    AffineTransformation::pointer finalTransform;
    Matrix4f finalTransformMatrix;
    {
        Matrix4f scaleMatrix = scaleTransform->matrix();
        Vector4f s0 = scaleMatrix.row(0);
        Vector4f s1 = scaleMatrix.row(1);
        Vector4f s2 = scaleMatrix.row(2);
        Vector4f s3 = scaleMatrix.row(3);
        addTransformationToFrame(rootFrame, scaleTransform);
        //Init
        BoundingBox box3 = rootFrame->getTransformedBoundingBox();
        Vector3f corner3 = box3.getCorners().row(0);
        minCoords(0) = corner3(0);
        minCoords(1) = corner3(1);
        minCoords(2) = corner3(2);
        maxCoords(0) = corner3(0);
        maxCoords(1) = corner3(1);
        maxCoords(2) = corner3(2);
        //Define lists to store results for each frame
        frameMinList = {}; // std::vector global Vector3f
        frameMaxList = {}; // std::vector global Vector3f
        frameBaseCornerList = {}; // std::vector global Vector3f
        framePlaneNormalList = {}; // std::vector global Vector3f
        frameInverseTransformList = {}; // std::vector global AffineTransformation::pointer
        framePlaneDValueList = {}; // std::vector global floats
        for (int i = 0; i < frameList.size(); i++){
            Image::pointer frame = frameList[i];
            // Start transforming frame
            if (i != 0)
                addTransformationToFrame(frame, scaleTransform);
            // Check corners min/max of frame
            // If not square has to use pixel points and AffineTransformation.. And more soffisticated method
            BoundingBox box = frame->getTransformedBoundingBox();
            MatrixXf corners = box.getCorners();
            Vector3f baseCorner = corners.row(0);
            Vector3f minCoordsFrame = baseCorner;
            Vector3f maxCoordsFrame = baseCorner;
            for (int j = 0; j < 8; j++){
                for (int k = 0; k < 3; k++){
                    float pointValue = corners(j, k);
                    if (pointValue < minCoordsFrame(k))
                        minCoordsFrame(k) = pointValue;
                    if (pointValue > maxCoordsFrame(k))
                        maxCoordsFrame(k) = pointValue;
                }
            }
            for (int k = 0; k < 3; k++){
                if (minCoordsFrame(k) < minCoords(k))
                    minCoords(k) = minCoordsFrame(k);
                if (maxCoordsFrame(k) > maxCoords(k))
                    maxCoords(k) = maxCoordsFrame(k);
            }
            // Calc plane values to store
            Vector3f framePlaneNormal = getImagePlaneNormal(frame);
            float framePlaneDvalue = calculatePlaneDvalue(baseCorner, framePlaneNormal);
            AffineTransformation::pointer frameInverseTransform = getInverseTransformation(frame);
            Matrix4f inverseTransformMatrix = frameInverseTransform->matrix();
            Vector4f i0 = inverseTransformMatrix.row(0);
            Vector4f i1 = inverseTransformMatrix.row(1);
            Vector4f i2 = inverseTransformMatrix.row(2);
            Vector4f i3 = inverseTransformMatrix.row(3);
            // Store frame values for later
            frameMinList.push_back(minCoordsFrame);
            frameMaxList.push_back(maxCoordsFrame);
            frameBaseCornerList.push_back(baseCorner);
            framePlaneNormalList.push_back(framePlaneNormal);
            frameInverseTransformList.push_back(frameInverseTransform);// getInverseTransformation(frame));
            framePlaneDValueList.push_back(framePlaneDvalue);
        }

        //Test total results: inverseSystemTransform & transformToMinimum & scaleTransform
        //AffineTransformation::pointer totalTransform = transformToMinimum->multiply(inverseSystemTransform);
        finalTransform = scaleTransform->multiply(totalTransform);// (scaleTransform->multiply(transformToMinimum))->multiply(inverseSystemTransform);// totalTransform);
        finalTransformMatrix = finalTransform->matrix();
        Vector4f f0 = finalTransformMatrix.row(0);
        Vector4f f1 = finalTransformMatrix.row(1);
        Vector4f f2 = finalTransformMatrix.row(2);
        Vector4f f3 = finalTransformMatrix.row(3);
        //Print final transform matrix
        std::cout << " ## Final transform matrix: ## " << std::endl;
        std::cout << "________________________________________" << std::endl;
        std::cout << "| " << f0(0) << " | " << f0(1) << " | " << f0(2) << " | " << f0(3) << " |" << std::endl;
        std::cout << "| " << f1(0) << " | " << f1(1) << " | " << f1(2) << " | " << f1(3) << " |" << std::endl;
        std::cout << "| " << f2(0) << " | " << f2(1) << " | " << f2(2) << " | " << f2(3) << " |" << std::endl;
        std::cout << "| " << f3(0) << " | " << f3(1) << " | " << f3(2) << " | " << f3(3) << " |" << std::endl;
        std::cout << "________________________________________" << std::endl;
        std::cout << " ## End final transform matrix: ## " << std::endl;
    }
    

    //Test frame 10
    AffineTransformation::pointer testTransformation3 = SceneGraph::getAffineTransformationFromData(testFrame);
    pointZero = testTransformation3->multiply(Vector3f(0, 0, 0));
    pointHero = testTransformation2->multiply(Vector3f(350, 546, 0));
    Matrix4f testMatrix3 = testTransformation3->matrix();
    t0 = testMatrix3.row(0);
    t1 = testMatrix3.row(1);
    t2 = testMatrix3.row(2);
    t3 = testMatrix3.row(3);
    AffineTransformation::pointer testTransformationInverse = testTransformation3->getInverse();
    Vector3f pointVolZero = testTransformationInverse->multiply(Vector3f(0, 0, 0));
    Vector3f pointVolHero = testTransformationInverse->multiply(Vector3f(100, 100, 10));
    Matrix4f testMatrix4 = testTransformationInverse->matrix();
    t0 = testMatrix4.row(0);
    t1 = testMatrix4.row(1);
    t2 = testMatrix4.row(2);
    t3 = testMatrix4.row(3);
    tFoo = false;

    // FINAL CALCULATIONS & TEST
    Vector3f size;
    {
        //finalTransformMatrix/finalTransform
        //totalTransform/totalMatrix
        //Vector4f point0f = totalMatrix * Vector4f(0, 0, 0, 1);
        //Vector4f pointEndf = totalMatrix * Vector4f(280, 400, 0, 1);
        pointZero = totalTransform->multiply(Vector3f(0, 0, 0));
        pointHero = totalTransform->multiply(Vector3f(350, 546, 0));
        // Init volume of size max - min in each direction x / y / z
        size = maxCoords - minCoords;
        volumeSize = Vector3i(ceil(size(0)) + 1, ceil(size(1)) + 1, ceil(size(2)) + 1);
        Vector3f pointVolZero = testTransformationInverse->multiply(Vector3f(0, 0, 0));
        Vector3f pointVolHero = testTransformationInverse->multiply(size);
        std::cout << "Final volumeSize:\n" << volumeSize << std::endl;

        //Calculate DV
        float zSize = volumeSize.z();
        int framesTot = frameList.size();
        float multiplicator = 1.0f;
        float calcDV = (round(100.0f  * multiplicator * ((zSize-1) / (float)(framesTot))) / 100.0f); //* 0.8f
        float calcRmax = calcDV * 8.0f * multiplicator;
        std::cout << "calcDV: " << calcDV << std::endl;
        std::cout << "calcRmax: " << calcRmax << std::endl;
        bool good = false;
    }

    // MAKE VOLUME
    DataType type = DataType::TYPE_FLOAT; //Endre til INT på sikt?
    int nrOfComponents = 2; // pixelvalues & weights
    AccumulationVolume = Image::New();
    AccumulationVolume->create(volumeSize(0), volumeSize(1), volumeSize(2), type, nrOfComponents);

    // INITIALIZE VOLUME
    float initVal = 0.0; 
    if (false && !runAsVNNonly){
        //TODOOOO
        //Init volume to zero values and two components
        std::cout << "Beginning volume zero initialization("<<volumeSize(0)<<"-"<<volumeSize(1)<<"-"<<volumeSize(2)<<")." << std::endl;
        volAccess = AccumulationVolume->getImageAccess(accessType::ACCESS_READ_WRITE); //global volAccess ImageAccess::pointer
        //T * inputData = (T*)inputAccess->get();
        //T * outputData = (T*)outputAccess->get();
        //float * outputData = (float*)volAccess->get();
        int width = volumeSize.x();
        int height = volumeSize.y();
        int depth = volumeSize.z();
        for (int x = 0; x < width; x++){
            for (int y = 0; y < height; y++){
                for (int z = 0; z < depth; z++){
                    Vector3i location = Vector3i(x, y, z);
                    //int loc = x*nrOfComponents + y*nrOfComponents*width + z*nrOfComponents*width*height;
                    //outputData[loc] = initVal;
                    //outputData[loc + 1] = initVal;
                    volAccess->setScalar(location, initVal, 0); //Channel 1 - Value
                    volAccess->setScalar(location, initVal, 1); //Channel 2 - Weight
                }
            }
            std::cout << ".";
        }
        std::cout << "FINISHED!" << std::endl;
        volAccess->release();
    }
    //Init dv (based on input frames/userdefined settings?)
    //TODO

}

void Us3Dhybrid::execute(){
    if (!reachedEndOfStream){
        std::cout << "Iteration #:" << iterartorCounter++ << std::endl;
        Image::pointer frame = getStaticInputData<Image>(0);
        frameList.push_back(frame);
        if (!firstFrameSet){
            firstFrame = frame;
            firstFrameSet = true;
        }
        // Sjekk om vi har nådd slutten
        DynamicData::pointer dynamicImage = getInputData(0);
        if (dynamicImage->hasReachedEnd()) {
            reachedEndOfStream = true;
        }
        //mIsModified = true;
        //setStaticOutputData<Image>(0, frame);
    }
    // When we have reached the end of stream we do just from here on
    if (reachedEndOfStream) {
        std::cout << "END Iteration #:" << iterartorCounter++ << std::endl;
        //mIsModified = false;
        if (!volumeCalculated){
            if (!volumeInitialized){
                std::cout << "Nr of frames in frameList:" << frameList.size() << std::endl;
                std::cout << "INITIALIZING volume" << std::endl;
                //Init cube with all corners
                initVolume(firstFrame);
                //zeroInitVolume();
                volumeInitialized = true;
                //Definer dv (oppløsning)
                //dv = 1; //ev egen function to define DV
                //outputImg = firstFrame;
            }
            if (!runAsVNNonly){
                executeAlgorithm();
                generateOutputVolume(); //Alternatively just fetch slices
            }
            else {
                executeVNN();
            }
            
            volumeCalculated = true;
            mIsModified = true;
            std::cout << "Finished!!!" << std::endl;
        }
        //setStaticOutputData<Image>(0, outputVolume);        
        //TODO add these?
        //SceneGraph::setParentNode(outputVolume, frame);
        volAccess->release();
    }
}



