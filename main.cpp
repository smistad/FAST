#include "OpenCLManager.hpp"
#include "ImageImporter2D.hpp"
using namespace fast;


int main(int argc, char ** argv) {

    oul::DeviceCriteria criteria;
    criteria.setDeviceCountCriteria(1);
    criteria.setTypeCriteria(oul::DEVICE_TYPE_GPU);
    oul::OpenCLManager * manager = oul::OpenCLManager::getInstance();
    oul::Context context = manager->createContext(criteria);

    ImageImporter2D::Ptr importer = ImageImporter2D::New();
    importer->setFilename("lena.jpg");
    importer->setContext(context);
    Image2D::Ptr image = importer->getOutput();
    std::cout << "Pipeline setup finished" << std::endl;
    image->update();
    std::cout << "Pipeline finished execution" << std::endl;
}
