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

void VTKImageExporter::SetInput(Image2D::pointer image) {
    mInput = image;
}

VTKImageExporter::VTKImageExporter() {
    // VTK stuff
    this->SetNumberOfOutputPorts(1);
    this->SetNumberOfInputPorts(0);
}

int VTKImageExporter::RequestData(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector) {

    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    vtkImageData *output = vtkImageData::SafeDownCast(
            outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    //image->SetDimensions(2,3,1);
    image->SetExtent(0, mInput->getWidth(), 0, mInput->getHeight(), 0, 0);
    image->SetScalarType(VTK_FLOAT);

    // Transfer data from mInput to image
    // TODO support different data types
    //float * vtkPixelData = (float *)image->GetScalarPointer();

    mInput->update(); // Make sure input is up to date
    ImageAccess2D access = mInput->getImageAccess(ACCESS_READ);
    float * fastPixelData = (float *)access.get();
    for(unsigned int x = 0; x < mInput->getWidth(); x++) {
    for(unsigned int y = 0; y < mInput->getHeight(); y++) {
        float * pixel = static_cast<float*>(image->GetScalarPointer(x,mInput->getHeight()-y,0));
        pixel[0] = fastPixelData[x+y*mInput->getWidth()];
    }}

    output->ShallowCopy(image);

    // Without these lines, the output will appear real but will not work as the input to any other filters
    output->SetExtent(image->GetExtent());
    output->SetUpdateExtent(output->GetExtent());
    output->SetWholeExtent(output->GetExtent());

    return 1;
}
