#include "SmartPointers.hpp"
#include "Exception.hpp"
#include "ImageImporter2D.hpp"
#include "ImageExporter2D.hpp"
#include "VTKImageExporter.hpp"
#include "VTKImageImporter.hpp"
#include "ITKImageExporter.hpp"
#include "ImageStreamer2D.hpp"
#include "DeviceManager.hpp"
#include "GaussianSmoothingFilter2D.hpp"
#include "SimpleWindow.hpp"
#include "ImageRenderer.hpp"

#include <vtkVersion.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageMapper.h>
#include <vtkActor2D.h>

#include <QApplication>

using namespace fast;

Image2D::pointer create() {
    // Example of importing one 2D image
    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    return importer->getOutput();
}

int main(int argc, char ** argv) {

    // Get a GPU device and set it as the default device
    DeviceManager& deviceManager = DeviceManager::getInstance();
    deviceManager.setDefaultDevice(deviceManager.getOneGPUDevice(true));




    // Example of importing, processing and exporting a 2D image
    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    GaussianSmoothingFilter2D::pointer filter = GaussianSmoothingFilter2D::New();
    filter->setInput(importer->getOutput());
    filter->setMaskSize(7);
    filter->setStandardDeviation(10);
    Image2D::pointer filteredImage = filter->getOutput();
    ImageExporter2D::pointer exporter = ImageExporter2D::New();
    exporter->setFilename("test.jpg");
    exporter->setInput(filteredImage);
    exporter->update();




    // Example of displaying an image on screen using ImageRenderer (2D) and SimpleWindow
    // TODO The QApplication part should ideally be hid away
    QApplication app(argc,argv);
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInput(filteredImage);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->resize(512,512);
    window->runMainLoop();




    // Example of creating a pipeline in another scope and updating afterwards
    Image2D::pointer image2 = create();
    std::cout << "after create" << std::endl;
    image2->update();





    // Example of streaming 2D images
    ImageStreamer2D::pointer streamer = ImageStreamer2D::New();
    streamer->setFilenameFormat("test_#.jpg");
    GaussianSmoothingFilter2D::pointer filter2 = GaussianSmoothingFilter2D::New();
    filter2->setInput(streamer->getOutput());
    filter2->setMaskSize(7);
    filter2->setStandardDeviation(10);
    Image2Dt::pointer dynamicImage = filter2->getOutput();
    // Call update 4 times
    int i = 4;
    while(--i) {
        dynamicImage->update();
    }

}
