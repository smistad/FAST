#include "ImageImporter2D.hpp"
#include "DeviceManager.hpp"
using namespace fast;


int main(int argc, char ** argv) {

    // Get a GPU device and set it as the default device
    DeviceManager& deviceManager = DeviceManager::getInstance();
    deviceManager.setDefaultDevice(deviceManager.getOneGPUDevice());

    ImageImporter2D::Ptr importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    Image2D::Ptr image = importer->getOutput();
    std::cout << "Pipeline setup finished" << std::endl;
    image->update();
    std::cout << "Pipeline finished execution" << std::endl;
}
