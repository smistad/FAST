#include "ImageImporter2D.hpp"
#include "ImageExporter2D.hpp"
#include "ImageStreamer2D.hpp"
#include "DeviceManager.hpp"
#include "GaussianSmoothingFilter2D.hpp"
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
}
