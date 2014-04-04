#include "catch.hpp"

#include "DeviceManager.hpp"
#include "ImageImporter2D.hpp"
#include "VTKImageImporter.hpp"
#include "VTKImageExporter.hpp"

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

TEST_CASE("Import an image from VTK to FAST", "[fast]") {
    // Get a GPU device and set it as the default device
    DeviceManager& deviceManager = DeviceManager::getInstance();
    deviceManager.setDefaultDevice(deviceManager.getOneGPUDevice(false));

    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    Image2D::pointer fastImage = importer->getOutput();

    // VTK Export
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->SetInput(fastImage);
    vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
    vtkExporter->Update();

    // VTK Import example
    VTKImageImporter::pointer vtkImporter = VTKImageImporter::New();
    vtkImporter->setInput(vtkImage);
    Image2D::pointer importedImage = vtkImporter->getOutput();
    vtkImporter->update();

    CHECK(fastImage->getWidth() == importedImage->getWidth());
    CHECK(fastImage->getHeight() == importedImage->getHeight());
}


TEST_CASE("Export an image from FAST to VTK", "[fast]") {
    // Get a GPU device and set it as the default device
    DeviceManager& deviceManager = DeviceManager::getInstance();
    deviceManager.setDefaultDevice(deviceManager.getOneGPUDevice(false));

    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    Image2D::pointer fastImage = importer->getOutput();

    CHECK_NOTHROW(
    // VTK Export and render example
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->SetInput(fastImage);
    vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
    vtkExporter->Update();

    // Check if images have same size
    int * size = vtkImage->GetDimensions();
    CHECK(fastImage->getWidth() == size[0]-1);
    CHECK(fastImage->getHeight() == size[1]-1);
    );
}

TEST_CASE("Export an image from FAST to VTK and visualize", "[fast]") {

    // Get a GPU device and set it as the default device
    DeviceManager& deviceManager = DeviceManager::getInstance();
    deviceManager.setDefaultDevice(deviceManager.getOneGPUDevice(false));

    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    Image2D::pointer fastImage = importer->getOutput();

    // VTK Export and render example
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->SetInput(fastImage);
    vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
    vtkExporter->Update();

    // Check if images have same size
    int * size = vtkImage->GetDimensions();
    CHECK(fastImage->getWidth() == size[0]-1);
    CHECK(fastImage->getHeight() == size[1]-1);

    CHECK_NOTHROW(
    // VTK mess for getting the image on screen
    vtkSmartPointer<vtkImageMapper> imageMapper = vtkSmartPointer<vtkImageMapper>::New();
#if VTK_MAJOR_VERSION <= 5
    imageMapper->SetInputConnection(vtkImage->GetProducerPort());
#else
    imageMapper->SetInputData(vtkImage);
#endif
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
