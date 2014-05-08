#include "VTKImageExporter.hpp"

#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkDataObject.h>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>

using namespace fast;

vtkStandardNewMacro(VTKImageExporter);

void VTKImageExporter::SetInput(Image::pointer image) {
    mInput = image;
}

VTKImageExporter::VTKImageExporter() {
    // VTK stuff
    this->SetNumberOfOutputPorts(1);
    this->SetNumberOfInputPorts(0);
}

template <class T>
void transferDataToVTKImage(Image::pointer input, vtkSmartPointer<vtkImageData> output) {
    ImageAccess access = input->getImageAccess(ACCESS_READ);
    T* fastPixelData = (T*)access.get();

    unsigned int width = input->getWidth();
    unsigned int height = input->getHeight();
    if(input->getDimensions() == 2) {
        for(unsigned int x = 0; x < width; x++) {
        for(unsigned int y = 0; y < height; y++) {
            T * pixel = static_cast<T*>(output->GetScalarPointer(x,height-y,0));
            pixel[0] = fastPixelData[x+y*width];
        }}
    } else {
        unsigned int depth = input->getDepth();
        for(unsigned int x = 0; x < width; x++) {
        for(unsigned int y = 0; y < height; y++) {
        for(unsigned int z = 0; z < depth; z++) {
            // TODO check the addressing here
            T * pixel = static_cast<T*>(output->GetScalarPointer(x,height-y,z));
            pixel[0] = fastPixelData[x+y*width+z*width*height];
        }}}
    }
}

int VTKImageExporter::RequestData(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector) {

    if(!mInput.isValid()) {
        throw Exception("No input supplied to GaussianSmoothingFilter");
    }
    mInput->update(); // Make sure input is up to date

    if(mInput->getNrOfComponents() != 1) {
        throw Exception("The VTKImageExporter currently doesn't support images with multiple components.");
    }

    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    vtkImageData *output = vtkImageData::SafeDownCast(
            outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();

    // Set size
    if(mInput->getDimensions() == 2) {
        image->SetExtent(0, mInput->getWidth(), 0, mInput->getHeight(), 0, 0);
    } else {
        image->SetExtent(0, mInput->getWidth(), 0, mInput->getHeight(), 0, mInput->getDepth());
    }

    // Set type
    switch(mInput->getDataType()) {
    case TYPE_FLOAT:
        image->SetScalarType(VTK_FLOAT);
        break;
    case TYPE_INT8:
        image->SetScalarType(VTK_CHAR);
        break;
    case TYPE_UINT8:
        image->SetScalarType(VTK_UNSIGNED_CHAR);
        break;
    case TYPE_INT16:
        image->SetScalarType(VTK_SHORT);
        break;
    case TYPE_UINT16:
        image->SetScalarType(VTK_UNSIGNED_SHORT);
        break;
    }

    // Transfer data from mInput to vtk image
    switch(mInput->getDataType()) {
        fastSwitchTypeMacro(transferDataToVTKImage<FAST_TYPE>(mInput, image))
    }

    output->ShallowCopy(image);

    // Without these lines, the output will appear real but will not work as the input to any other filters
    output->SetExtent(image->GetExtent());
    output->SetUpdateExtent(output->GetExtent());
    output->SetWholeExtent(output->GetExtent());

    return 1;
}
