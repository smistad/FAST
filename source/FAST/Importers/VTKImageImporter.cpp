#include "VTKImageImporter.hpp"
#include <vtkType.h>
using namespace fast;

void VTKImageImporter::setInput(vtkSmartPointer<vtkImageData> image) {
    mInput = image;
    mIsModified = true;
}

VTKImageImporter::VTKImageImporter() {
    createOutputPort<Image>(0, OUTPUT_STATIC);
}

template <class T>
void* readVTKData(vtkSmartPointer<vtkImageData> image) {
    // TODO component support
    int * size = image->GetDimensions();
    unsigned int width = size[0]-1;
    unsigned int height = size[1]-1;
    if(image->GetDataDimension() == 2) {
        T* fastPixelData = new T[width*height];
        for(unsigned int x = 0; x < width; x++) {
        for(unsigned int y = 0; y < height; y++) {
            T * pixel = static_cast<T*>(image->GetScalarPointer(x,height-y,0));
            fastPixelData[x+y*width] = pixel[0];
        }}
        return (void*)fastPixelData;
    } else if(image->GetDataDimension() == 3) {
        unsigned int depth = size[2]-1;
        T* fastPixelData = new T[width*height*depth];
        for(unsigned int x = 0; x < width; x++) {
        for(unsigned int y = 0; y < height; y++) {
        for(unsigned int z = 0; z < depth; z++) {
            T * pixel = static_cast<T*>(image->GetScalarPointer(x,height-y,z));
            fastPixelData[x+y*width+z*width*height] = pixel[0];
        }}}
        return (void*)fastPixelData;
    } else {
        throw Exception("Wrong number of dimensions in VTK image");
        return NULL;
    }
}

void transferVTKDataToFAST(vtkSmartPointer<vtkImageData> image, Image::pointer output) {

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
        output->create2DImage(size[0]-1, size[1]-1,type,1,Host::getInstance(),data);
    } else if(image->GetDataDimension() == 3) {
        output->create3DImage(size[0]-1, size[1]-1,size[2]-1,type,1,Host::getInstance(),data);
    } else {
        throw Exception("Wrong number of dimensions in VTK image");
    }
    deleteArray(data, type);
    output->updateModifiedTimestamp();
}

void VTKImageImporter::execute() {
    // Make sure VTK data is up to date
#if VTK_MAJOR_VERSION <= 5
    mInput->Update();
#else
#endif

    transferVTKDataToFAST(mInput, getOutputData<Image>());

}
