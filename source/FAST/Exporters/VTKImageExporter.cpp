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

VTKImageExporter::VTKImageExporter() {
    // VTK stuff
    this->SetNumberOfOutputPorts(1);
    this->SetNumberOfInputPorts(0);
    createInputPort<Image>(0);
}

template <class T>
void transferDataToVTKImage(Image::pointer input, vtkSmartPointer<vtkImageData> output) {
    ImageAccess::pointer access = input->getImageAccess(ACCESS_READ);
    T* fastPixelData = (T*)access->get();

    int width = input->getWidth();
    int height = input->getHeight();
    if(input->getDimensions() == 2) {
        for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            T * pixel = static_cast<T*>(output->GetScalarPointer(x,y,0));
            pixel[0] = fastPixelData[x+y*width];
        }}
    } else {
        int depth = input->getDepth();
        for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
        for(int z = 0; z < depth; z++) {
            // TODO check the addressing here
            T * pixel = static_cast<T*>(output->GetScalarPointer(x,y,z));
            pixel[0] = fastPixelData[x+y*width+z*width*height];
        }}}
    }
}

int VTKImageExporter::RequestData(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector) {

    update(0); // Run FAST pipeline

    Image::pointer input = getInputData<Image>();

    if(input->getNrOfChannels() != 1)
        throw Exception("The VTKImageExporter currently doesn't support images with multiple components.");

    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    vtkImageData *output = this->GetOutput();

    // Set size
    if(input->getDimensions() == 2) {
        output->SetExtent(0, input->getWidth()-1, 0, input->getHeight()-1, 0, 0);
    } else {
        output->SetExtent(0, input->getWidth()-1, 0, input->getHeight()-1, 0, input->getDepth()-1);
    }

    output->SetSpacing(input->getSpacing().x(), input->getSpacing().y(), input->getSpacing().z());

#if VTK_MAJOR_VERSION <= 5
    output->SetNumberOfScalarComponents(1);
    // Set type
    switch(input->getDataType()) {
    case TYPE_FLOAT:
        output->SetScalarType(VTK_FLOAT);
        break;
    case TYPE_INT8:
        output->SetScalarType(VTK_CHAR);
        break;
    case TYPE_UINT8:
        output->SetScalarType(VTK_UNSIGNED_CHAR);
        break;
    case TYPE_INT16:
        output->SetScalarType(VTK_SHORT);
        break;
    case TYPE_UINT16:
        output->SetScalarType(VTK_UNSIGNED_SHORT);
        break;
    default:
        throw Exception("Unknown type");
        break;
    }
#else
    // Set type
    switch(input->getDataType()) {
    case TYPE_FLOAT:
        output->AllocateScalars(VTK_FLOAT, 1);
        break;
    case TYPE_INT8:
        output->AllocateScalars(VTK_CHAR, 1);
        break;
    case TYPE_UINT8:
        output->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
        break;
    case TYPE_INT16:
        output->AllocateScalars(VTK_SHORT, 1);
        break;
    case TYPE_UINT16:
        output->AllocateScalars(VTK_UNSIGNED_SHORT, 1);
        break;
    default:
        throw Exception("Unknown type");
        break;
    }
#endif


    // Transfer data from mInput to vtk image
    switch(input->getDataType()) {
        fastSwitchTypeMacro(transferDataToVTKImage<FAST_TYPE>(input, output))
    }

    // Without these lines, the output will appear real but will not work as the input to any other filters
    //output->SetExtent(output->GetExtent());
#if VTK_MAJOR_VERSION <= 5
    //output->SetUpdateExtent(output->GetExtent());
    //output->SetWholeExtent(output->GetExtent());
#endif

    return 1;
}
