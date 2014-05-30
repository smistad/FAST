#include "catch.hpp"

#include "DeviceManager.hpp"
#include "ImageImporter.hpp"
#include "VTKImageExporter.hpp"
#include "Image.hpp"

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

TEST_CASE("Export a 2D image from FAST to VTK", "[fast][VTK]") {
    unsigned int width = 32;
    unsigned int height = 40;
    for(unsigned int typeNr = 0; typeNr < 5; typeNr++) { // for all types
        Image::pointer fastImage = Image::New();
        DataType type = (DataType)typeNr;
        fastImage->create2DImage(width, height, type, 1, Host::New());

        vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
        vtkExporter->SetInput(fastImage);
        vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
        vtkExporter->Update();

        // Check if images have same size
        int * size = vtkImage->GetDimensions();
        CHECK(width == size[0]-1);
        CHECK(height == size[1]-1);
        CHECK(2 == vtkImage->GetDataDimension());
        CHECK(vtkImage->GetScalarType() == getVTKTypeFromDataType(type));
    }
}

TEST_CASE("Export a 3D image from FAST to VTK", "[fast][VTK]") {
    unsigned int width = 32;
    unsigned int height = 20;
    unsigned int depth = 8;
    for(unsigned int typeNr = 0; typeNr < 5; typeNr++) { // for all types
        Image::pointer fastImage = Image::New();
        DataType type = (DataType)typeNr;
        fastImage->create3DImage(width, height, depth, type, 1, Host::New());

        vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
        vtkExporter->SetInput(fastImage);
        vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
        vtkExporter->Update();

        // Check if images have same size
        int * size = vtkImage->GetDimensions();
        CHECK(width == size[0]-1);
        CHECK(height == size[1]-1);
        CHECK(depth == size[2]-1);
        CHECK(3 == vtkImage->GetDataDimension());
        CHECK(vtkImage->GetScalarType() == getVTKTypeFromDataType(type));
    }
}

TEST_CASE("Export an image from FAST to VTK and visualize", "[fast][VTK]") {

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "lena.jpg");
    Image::pointer fastImage = importer->getOutput();

    // VTK Export and render example
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->SetInput(fastImage);
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
