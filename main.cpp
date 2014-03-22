#include "ImageImporter2D.hpp"
#include "ImageStreamer2D.hpp"
#include "DeviceManager.hpp"
using namespace fast;

int main(int argc, char ** argv) {

    // Get a GPU device and set it as the default device
    DeviceManager& deviceManager = DeviceManager::getInstance();
    deviceManager.setDefaultDevice(deviceManager.getOneGPUDevice());

    // Example of importing one 2D image
    ImageImporter2D::Ptr importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    Image2D::Ptr image = importer->getOutput();
    image->update();

    // Example of streaming 2D images
    ImageStreamer2D::Ptr streamer = ImageStreamer2D::New();
    streamer->setFilenameFormat("test_#.jpg");
    Image2Dt::Ptr dynamicImage = streamer->getOutput();
    dynamicImage->update();
}
