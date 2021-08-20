#include "FAST/Testing.hpp"
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

TEST_CASE("No input given to the VTKImageExporter", "[fast][VTK][VTKImageExporter]") {
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
    CHECK_THROWS(vtkExporter->Update());
}

TEST_CASE("Export a 2D image from FAST to VTK", "[fast][VTK][VTKImageExporter]") {
    int width = 32;
    int height = 40;
    for(int typeNr = 0; typeNr < 5; typeNr++) { // for all types
        DataType type = (DataType)typeNr;
        void* data = allocateRandomData(width*height, type);
        Image::pointer fastImage = Image::create(width, height, type, 1, Host::getInstance(), data);

        vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
        vtkExporter->setInputData(fastImage);
        vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
        vtkExporter->Update();

        // Check that vtk image has correct metadata
        int * size = vtkImage->GetDimensions();
        CHECK(width == size[0]);
        CHECK(height == size[1]);
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

TEST_CASE("Export a 3D image from FAST to VTK", "[fast][VTK][VTKImageExporter]") {
    int width = 32;
    int height = 20;
    int depth = 8;
    for(int typeNr = 0; typeNr < 5; typeNr++) { // for all types
        DataType type = (DataType)typeNr;
        void* data = allocateRandomData(width*height*depth, type);
        Image::pointer fastImage = Image::create(width, height, depth, type, 1, Host::getInstance(), data);

        vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
        vtkExporter->setInputData(fastImage);
        vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
        vtkExporter->Update();

        // Check that vtk image has correct metadata
        int * size = vtkImage->GetDimensions();
        CHECK(width == size[0]);
        CHECK(height == size[1]);
        CHECK(depth == size[2]);
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

/*
TEST_CASE("Export an image from FAST to VTK and visualize", "[fast][VTK][VTKImageExporter]") {

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/US-2D.jpg");
    DataChannel::pointer port = importer->getOutputPort();
    Image::pointer fastImage = port->getNextFrame();

    // VTK Export and render example
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->setInputConnection(importer->getOutputPort());
    vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
    vtkExporter->Update();

    // Check if images have same size
    int * size = vtkImage->GetDimensions();
    CHECK(fastImage->getWidth() == size[0]);
    CHECK(fastImage->getHeight() == size[1]);


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
    renderWindowInteractor->Start(); // this will block
    );
}
*/
