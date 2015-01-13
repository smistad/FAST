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

    update(); // Run FAST pipeline

    Image::pointer input = getStaticInputData<Image>();

    if(input->getNrOfComponents() != 1)
        throw Exception("The VTKImageExporter currently doesn't support images with multiple components.");

    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    vtkImageData *output = this->GetOutput();

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();

    // Set size
    if(input->getDimensions() == 2) {
        image->SetExtent(0, input->getWidth(), 0, input->getHeight(), 0, 0);
    } else {
        image->SetExtent(0, input->getWidth(), 0, input->getHeight(), 0, input->getDepth());
    }

#if VTK_MAJOR_VERSION <= 5
    // Set type
    switch(input->getDataType()) {
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
    default:
        throw Exception("Unknown type");
        break;
    }
#else
    // Set type
    switch(input->getDataType()) {
    case TYPE_FLOAT:
        image->SetScalarType(VTK_FLOAT, outInfo);
        break;
    case TYPE_INT8:
        image->SetScalarType(VTK_CHAR, outInfo);
        break;
    case TYPE_UINT8:
        image->SetScalarType(VTK_UNSIGNED_CHAR, outInfo);
        break;
    case TYPE_INT16:
        image->SetScalarType(VTK_SHORT, outInfo);
        break;
    case TYPE_UINT16:
        image->SetScalarType(VTK_UNSIGNED_SHORT, outInfo);
        break;
    default:
        throw Exception("Unknown type");
        break;
    }
#endif

    // Transfer data from mInput to vtk image
    switch(input->getDataType()) {
        fastSwitchTypeMacro(transferDataToVTKImage<FAST_TYPE>(input, image))
    }

    output->ShallowCopy(image);

    // Without these lines, the output will appear real but will not work as the input to any other filters
    output->SetExtent(image->GetExtent());
#if VTK_MAJOR_VERSION <= 5
    //output->SetUpdateExtent(output->GetExtent());
    //output->SetWholeExtent(output->GetExtent());
#endif

    return 1;
}
