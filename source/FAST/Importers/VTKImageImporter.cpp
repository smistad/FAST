#include "VTKImageImporter.hpp"
#include <vtkType.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>
using namespace fast;

vtkStandardNewMacro(VTKImageImporter);

VTKImageImporter::VTKImageImporter() {
    this->SetNumberOfOutputPorts(0);
    this->SetNumberOfInputPorts(1);
    createOutputPort<Image>(0);
    mIsModified = true;
}

template <class T>
void* readVTKData(vtkImageData* image) {
    // TODO component support
    int * size = image->GetDimensions();
    unsigned int width = size[0];
    unsigned int height = size[1];
    if(image->GetDataDimension() == 2) {
        T* fastPixelData = new T[width*height];
        for(unsigned int x = 0; x < width; x++) {
        for(unsigned int y = 0; y < height; y++) {
            T* pixel = static_cast<T*>(image->GetScalarPointer(x,height-y,0));
            fastPixelData[x+y*width] = pixel[0];
        }}
        return (void*)fastPixelData;
    } else if(image->GetDataDimension() == 3) {
        unsigned int depth = size[2];
        T* fastPixelData = new T[width*height*depth];
        for(unsigned int x = 0; x < width; x++) {
        for(unsigned int y = 0; y < height; y++) {
        for(unsigned int z = 0; z < depth; z++) {
            T*  pixel = static_cast<T*>(image->GetScalarPointer(x,height-y,z));
            fastPixelData[x+y*width+z*width*height] = pixel[0];
        }}}
        return (void*)fastPixelData;
    } else {
        throw Exception("Wrong number of dimensions in VTK image");
        return NULL;
    }
}
void transferVTKDataToFAST(vtkImageData* image, Image::pointer output) {

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
    if(image->GetDataDimension() == 2) {
        output->create(size[0], size[1],type,1,Host::getInstance(),data);
    } else if(image->GetDataDimension() == 3) {
        output->create(size[0], size[1],size[2],type,1,Host::getInstance(),data);
    } else {
        throw Exception("Wrong number of dimensions in VTK image");
    }
    deleteArray(data, type);
}


int VTKImageImporter::RequestData(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector) {
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    // Get the input data
    vtkImageData* input = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

    transferVTKDataToFAST(input, getOutputData<Image>());

    return 1;
}


void VTKImageImporter::execute() {
    // Run VTK pipeline
    Update();
}
