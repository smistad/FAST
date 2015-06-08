#include "FAST/Tests/catch.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Importers/ImageImporter.hpp"
#include "FAST/Exporters/VTKImageExporter.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Tests/DataComparison.hpp"

#include <vtkVersion.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageMapper.h>
#include <vtkActor2D.h>

using namespace fast;

inline int getVTKTypeFromDataType(const DataType type) {
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
inline bool compareVTKDataWithFASTData(vtkSmartPointer<vtkImageData> vtkImage, void* fastData) {
    int * size = vtkImage->GetDimensions();
    unsigned int width = size[0]-1;
    unsigned int height = size[1]-1;
    if(vtkImage->GetDataDimension() == 2) {
        for(unsigned int x = 0; x < width; x++) {
        for(unsigned int y = 0; y < height; y++) {
            T fastValue = ((T*)fastData)[x+y*width];
            T vtkValue = *(static_cast<T*>(vtkImage->GetScalarPointer(x,height-y,0)));
            if(fastValue != vtkValue) {
                return false;
            }
        }}
    } else {
        unsigned int depth = size[2]-1;
        for(unsigned int x = 0; x < width; x++) {
        for(unsigned int y = 0; y < height; y++) {
        for(unsigned int z = 0; z < depth; z++) {
            // TODO check the addressing here
            T fastValue = ((T*)fastData)[x+y*width+z*width*height];
            T vtkValue = *(static_cast<T*>(vtkImage->GetScalarPointer(x,height-y,z)));
            if(fastValue != vtkValue) {
                return false;
            }
        }}}
    }

    return true;
}

TEST_CASE("No input given to the VTKImageExporter", "[fast][VTK]") {
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
    CHECK_THROWS(vtkExporter->Update());
}

TEST_CASE("Export a 2D image from FAST to VTK", "[fast][VTK]") {
    unsigned int width = 32;
    unsigned int height = 40;
    for(unsigned int typeNr = 0; typeNr < 5; typeNr++) { // for all types
        Image::pointer fastImage = Image::New();
        DataType type = (DataType)typeNr;
        void* data = allocateRandomData(width*height, type);
        fastImage->create2DImage(width, height, type, 1, Host::getInstance(), data);

        vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
        vtkExporter->setInputData(fastImage);
        vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
        vtkExporter->Update();

        // Check that vtk image has correct metadata
        int * size = vtkImage->GetDimensions();
        CHECK(width == size[0]-1);
        CHECK(height == size[1]-1);
        CHECK(2 == vtkImage->GetDataDimension());
        CHECK(vtkImage->GetScalarType() == getVTKTypeFromDataType(type));

        // Check that vtk image has correct data
        bool success;
        switch(fastImage->getDataType()) {
            fastSwitchTypeMacro(success = compareVTKDataWithFASTData<FAST_TYPE>(vtkImage, data))
        }
        CHECK(success == true);
    }
}

TEST_CASE("Export a 3D image from FAST to VTK", "[fast][VTK]") {
    unsigned int width = 32;
    unsigned int height = 20;
    unsigned int depth = 8;
    for(unsigned int typeNr = 0; typeNr < 5; typeNr++) { // for all types
        Image::pointer fastImage = Image::New();
        DataType type = (DataType)typeNr;
        void* data = allocateRandomData(width*height*depth, type);
        fastImage->create3DImage(width, height, depth, type, 1, Host::getInstance(), data);

        vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
        vtkExporter->setInputData(fastImage);
        vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
        vtkExporter->Update();

        // Check that vtk image has correct metadata
        int * size = vtkImage->GetDimensions();
        CHECK(width == size[0]-1);
        CHECK(height == size[1]-1);
        CHECK(depth == size[2]-1);
        CHECK(3 == vtkImage->GetDataDimension());
        CHECK(vtkImage->GetScalarType() == getVTKTypeFromDataType(type));

        // Check that vtk image has correct data
        bool success;
        switch(fastImage->getDataType()) {
            fastSwitchTypeMacro(success = compareVTKDataWithFASTData<FAST_TYPE>(vtkImage, data))
        }
        CHECK(success == true);
    }
}

TEST_CASE("Export an image from FAST to VTK and visualize", "[fast][VTK]") {

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2D.jpg");
    Image::pointer fastImage = importer->getOutputData<Image>();

    // VTK Export and render example
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->setInputData(fastImage);
    vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
    vtkExporter->Update();

    // Check if images have same size
    int * size = vtkImage->GetDimensions();
    CHECK(fastImage->getWidth() == size[0]-1);
    CHECK(fastImage->getHeight() == size[1]-1);


    // VTK mess for getting the image on screen
    vtkSmartPointer<vtkImageMapper> imageMapper = vtkSmartPointer<vtkImageMapper>::New();
#if VTK_MAJOR_VERSION <= 5
    imageMapper->SetInputConnection(vtkImage->GetProducerPort());
#else
    imageMapper->SetInputData(vtkImage);
#endif
	CHECK_NOTHROW(
    imageMapper->SetColorWindow(1);
    imageMapper->SetColorLevel(0.5);

    vtkSmartPointer<vtkActor2D> imageActor = vtkSmartPointer<vtkActor2D>::New();
    imageActor->SetMapper(imageMapper);

    // Setup renderers and render window
    vtkSmartPointer<vtkRenderer> renderer2 = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer2);
    renderWindow->SetSize(fastImage->getWidth(), fastImage->getHeight());

    // Setup render window interactor
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();

    vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
    renderWindowInteractor->SetInteractorStyle(style);
    renderWindowInteractor->SetRenderWindow(renderWindow);
    renderer2->AddActor2D(imageActor);
    renderWindow->Render();
    //renderWindowInteractor->Start();
    );
}
