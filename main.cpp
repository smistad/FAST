#include "SmartPointers.hpp"
#include "Exception.hpp"
#include "ImageImporter2D.hpp"
#include "ImageExporter2D.hpp"
#include "ImageStreamer2D.hpp"
#include "DeviceManager.hpp"
#include "GaussianSmoothingFilter.hpp"
#include "SimpleWindow.hpp"
#include "ImageRenderer.hpp"
#include "SliceRenderer.hpp"
#include "MetaImageImporter.hpp"
#include "MetaImageStreamer.hpp"

#include <QApplication>

using namespace fast;

int main(int argc, char ** argv) {

    // Get a GPU device and set it as the default device
    DeviceManager& deviceManager = DeviceManager::getInstance();
    deviceManager.setDefaultDevice(deviceManager.getOneGPUDevice(true));




    // Example of importing, processing and exporting a 2D image
    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setInput(importer->getOutput());
    filter->setMaskSize(7);
    filter->setStandardDeviation(10);
    Image::pointer filteredImage = filter->getOutput();
    ImageExporter2D::pointer exporter = ImageExporter2D::New();
    exporter->setFilename("test.jpg");
    exporter->setInput(filteredImage);
    exporter->update();



    // Example of displaying an image on screen using ImageRenderer (2D) and SimpleWindow
    // TODO The QApplication part should ideally be hid away
    QApplication app(argc,argv);
    /*
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInput(filteredImage);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->resize(512,512);
    //window->runMainLoop();
     */

    // Example of streaming 2D images
    ImageStreamer2D::pointer streamer = ImageStreamer2D::New();
    streamer->setFilenameFormat("test_#.jpg");
    GaussianSmoothingFilter::pointer filter2 = GaussianSmoothingFilter::New();
    filter2->setInput(streamer->getOutput());
    filter2->setMaskSize(7);
    filter2->setStandardDeviation(10);
    DynamicImage::pointer dynamicImage = filter2->getOutput();
    // Call update 4 times
    int i = 4;
    while(--i) {
        dynamicImage->update();
    }




    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat("/home/smistad/Patients/2013-08-22_10-36_Lab_4DTrack.cx3/US_Acq/US-Acq_01_20130822T111033/US-Acq_01_20130822T111033_ScanConverted_#.mhd");
    GaussianSmoothingFilter::pointer filter4 = GaussianSmoothingFilter::New();
    filter4->setInput(mhdStreamer->getOutput());
    filter4->setMaskSize(5);
    filter4->setStandardDeviation(10);
    filter4->enableRuntimeMeasurements();

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setInput(filter4->getOutput());
    renderer->enableRuntimeMeasurements();
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->resize(512,512);
    window->runMainLoop();

    renderer->getRuntime()->print();
    filter4->getRuntime()->print();
}
