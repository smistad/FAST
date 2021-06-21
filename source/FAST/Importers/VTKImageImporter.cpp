#include "VTKImageImporter.hpp"
#include <vtkType.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>
using namespace fast;

vtkStandardNewMacro(VTKtoFAST);

VTKtoFAST::VTKtoFAST() {
    this->SetNumberOfOutputPorts(0);
    this->SetNumberOfInputPorts(1);
}

VTKImageImporter::VTKImageImporter() {
    createOutputPort<Image>(0);
    mIsModified = true;
    mVTKProcessObject = VTKtoFAST::New();
}

template <class T>
void* readVTKData(vtkImageData* image) {
    // TODO component support
    int * size = image->GetDimensions();
    int width = size[0];
    int height = size[1];
    if(image->GetDataDimension() == 2) {
        T* fastPixelData = new T[width*height];
        for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            T* pixel = static_cast<T*>(image->GetScalarPointer(x,y,0));
            fastPixelData[x+y*width] = pixel[0];
        }}
        return (void*)fastPixelData;
    } else if(image->GetDataDimension() == 3) {
        int depth = size[2];
        T* fastPixelData = new T[width*height*depth];
        for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
        for(int z = 0; z < depth; z++) {
            T*  pixel = static_cast<T*>(image->GetScalarPointer(x,y,z));
            fastPixelData[x+y*width+z*width*height] = pixel[0];
        }}}
        return (void*)fastPixelData;
    } else {
        throw Exception("Wrong number of dimensions in VTK image");
        return NULL;
    }
}

Image::pointer transferVTKDataToFAST(vtkImageData* image) {

    void* data;
    DataType type;
    switch(image->GetScalarType()) {
    case VTK_FLOAT:
        data = readVTKData<float>(image);
        type = TYPE_FLOAT;
        break;
    case VTK_CHAR:
    case VTK_SIGNED_CHAR:
        data = readVTKData<char>(image);
        type = TYPE_INT8;
        break;
    case VTK_UNSIGNED_CHAR:
        data = readVTKData<uchar>(image);
        type = TYPE_UINT8;
        break;
    case VTK_SHORT:
        data = readVTKData<short>(image);
        type = TYPE_INT16;
        break;
    case VTK_UNSIGNED_SHORT:
        data = readVTKData<ushort>(image);
        type = TYPE_UINT16;
        break;
    default:
        throw Exception("VTK image of unsupported type was supplied to the VTKImageImporter");
        break;
    }

    int * size = image->GetDimensions();
    Image::pointer output;
    if(image->GetDataDimension() == 2) {
        output = Image::create(size[0], size[1],type,1,Host::getInstance(),data);
    } else if(image->GetDataDimension() == 3) {
        output = Image::create(size[0], size[1],size[2],type,1,Host::getInstance(),data);
    } else {
        throw Exception("Wrong number of dimensions in VTK image");
    }
    deleteArray(data, type);
    return output;
}


int VTKtoFAST::RequestData(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector) {
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    // Get the input data
    vtkImageData* input = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

    mFASTImage = transferVTKDataToFAST(input);

    return 1;
}

Image::pointer VTKtoFAST::getFASTImage() {
    return mFASTImage;
}

void VTKImageImporter::execute() {
    // Run VTK pipeline
    mVTKProcessObject->Update();
    auto image = mVTKProcessObject->getFASTImage();
    addOutputData(0, image);
}

VTKtoFAST *VTKImageImporter::getVTKProcessObject() {
    return mVTKProcessObject;
}
