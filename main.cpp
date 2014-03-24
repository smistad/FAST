#include "ImageImporter2D.hpp"
#include "ImageExporter2D.hpp"
#include "VTKImageExporter.hpp"
#include "ImageStreamer2D.hpp"
#include "DeviceManager.hpp"
#include "GaussianSmoothingFilter2D.hpp"

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

Image2D::Ptr create() {
    // Example of importing one 2D image
    ImageImporter2D::Ptr importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    return importer->getOutput();
}

int main(int argc, char ** argv) {

    // Get a GPU device and set it as the default device
    DeviceManager& deviceManager = DeviceManager::getInstance();
    deviceManager.setDefaultDevice(deviceManager.getOneGPUDevice());

    // Example of importing, processing and exporting a 2D image
    ImageImporter2D::Ptr importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    Image2D::Ptr image = importer->getOutput();
    GaussianSmoothingFilter2D::Ptr filter = GaussianSmoothingFilter2D::New();
    filter->setMaskSize(7);
    filter->setStandardDeviation(10);
    filter->setInput(image);
    Image2D::Ptr filteredImage = filter->getOutput();
    ImageExporter2D::Ptr exporter = ImageExporter2D::New();
    exporter->setFilename("test.jpg");
    exporter->setInput(filteredImage);
    exporter->update();

    // Example of creating a pipeline in another scope and updating afterwards
    Image2D::Ptr image2 = create();
    std::cout << "after create" << std::endl;
    image2->update();

    // Example of streaming 2D images
    ImageStreamer2D::Ptr streamer = ImageStreamer2D::New();
    streamer->setFilenameFormat("test_#.jpg");
    Image2Dt::Ptr dynamicImage = streamer->getOutput();
    dynamicImage->update();

    // VTK Export and render example
    VTKImageExporter::Ptr vtkExporter = VTKImageExporter::New();
    vtkExporter->setInput(filteredImage);
    vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
    vtkExporter->Update();

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

    // Setup renderers
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

    // Setup render window
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();

    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(filteredImage->getWidth(), filteredImage->getHeight());

    // Setup render window interactor
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();

    vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();

    renderWindowInteractor->SetInteractorStyle(style);

    // Render and start interaction
    renderWindowInteractor->SetRenderWindow(renderWindow);

    //renderer->AddViewProp(imageActor);
    renderer->AddActor2D(imageActor);

    renderWindow->Render();
    renderWindowInteractor->Start();
}
