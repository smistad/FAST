#include "FAST/Testing.hpp"
#include "FAST/Importers/VTKImageImporter.hpp"
#include "FAST/Exporters/VTKImageExporter.hpp"
#include "FAST/Importers/ImageImporter.hpp"
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <FAST/Tests/DataComparison.hpp>

using namespace fast;

// TODO rewrite this test so that it doesn't use the vtk exporter
TEST_CASE("Import an image from VTK to FAST", "[fast][VTK][VTKImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/US-2D.jpg");
    DataPort::pointer port = importer->getOutputPort();

    // VTK Export
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->setInputConnection(importer->getOutputPort());
    vtkExporter->Update();

    // VTK Import example
    VTKImageImporter::pointer vtkImporter = VTKImageImporter::New();
    VTKtoFAST* vtkPO = vtkImporter->getVTKProcessObject();
    vtkPO->SetInputConnection(vtkExporter->GetOutputPort());
    DataPort::pointer port2 = vtkImporter->getOutputPort();
    vtkImporter->update(0);

    Image::pointer importedImage = port2->getNextFrame<Image>();
    Image::pointer fastImage = port->getNextFrame<Image>();

    CHECK(importedImage->getWidth() == fastImage->getWidth());
    CHECK(importedImage->getHeight() == fastImage->getHeight());
    CHECK(importedImage->getDepth() == 1);
    CHECK(importedImage->getDimensions() == 2);
    CHECK(importedImage->getDataType() == TYPE_UINT8);
}

static int getVTKTypeFromDataType(const DataType type) {
    int result = 0;
    switch(type) {
    case TYPE_FLOAT:
        result = VTK_FLOAT;
        break;
    case TYPE_INT8:
        result = VTK_CHAR;
        break;
    case TYPE_UINT8:
        result = VTK_UNSIGNED_CHAR;
        break;
    case TYPE_INT16:
        result = VTK_SHORT;
        break;
    case TYPE_UINT16:
        result = VTK_UNSIGNED_SHORT;
        break;
    }

    return result;
}

template <class T>
static bool compareVTKDataWithFASTData(vtkSmartPointer<vtkImageData> vtkImage, void* fastData) {
    int * size = vtkImage->GetDimensions();
    int width = size[0];
    int height = size[1];
    if(vtkImage->GetDataDimension() == 2) {
        for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            T fastValue = ((T*)fastData)[x+y*width];
            T vtkValue = *(static_cast<T*>(vtkImage->GetScalarPointer(x,y,0)));
            if(fastValue != vtkValue) {
                return false;
            }
        }}
    } else {
        int depth = size[2];
        for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
        for(int z = 0; z < depth; z++) {
            // TODO check the addressing here
            T fastValue = ((T*)fastData)[x+y*width+z*width*height];
            T vtkValue = *(static_cast<T*>(vtkImage->GetScalarPointer(x,y,z)));
            if(fastValue != vtkValue) {
                return false;
            }
        }}}
    }

    return true;
}

template<class T>
static vtkImageData* createVTKImage(T* data, int width, int height, DataType type) {
    vtkImageData* output = vtkImageData::New();

    // Set size
    output->SetExtent(0, width-1, 0, height-1, 0, 0);
    // Set type
    output->AllocateScalars(getVTKTypeFromDataType(type), 1);

    for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
        T * pixel = static_cast<T*>(output->GetScalarPointer(x,y,0));
        pixel[0] = data[x+y*width];
    }}

    return output;
}

TEST_CASE("Import a 2D image from VTK to FAST", "[fast][VTK][VTKImageImporter]") {
    int width = 32;
    int height = 40;
    for(int typeNr = 0; typeNr < 5; typeNr++) { // for all types
        DataType type = (DataType)typeNr;
        void* data = allocateRandomData(width*height, type);
        vtkImageData* vtkImage;

        switch(type) {
            fastSwitchTypeMacro(vtkImage = createVTKImage<FAST_TYPE>((FAST_TYPE*)data, width, height, type);)
        }

        VTKImageImporter::pointer vtkImporter = VTKImageImporter::New();
        VTKtoFAST* vtkPO = vtkImporter->getVTKProcessObject();
        vtkPO->SetInputData(vtkImage);
        DataPort::pointer port = vtkImporter->getOutputPort();
        vtkImporter->update(0);

        Image::pointer importedImage = port->getNextFrame<Image>();
        ImageAccess::pointer access = importedImage->getImageAccess(ACCESS_READ);

        // Check that vtk image has correct data
        bool success;
        switch(importedImage->getDataType()) {
            fastSwitchTypeMacro(success = compareVTKDataWithFASTData<FAST_TYPE>(vtkImage, access->get()))
        }
        CHECK(success == true);
    }
}