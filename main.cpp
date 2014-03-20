#include "OpenCLManager.hpp"
#include "ImageImporter2D.hpp"
using namespace fast;


int main(int argc, char ** argv) {

    oul::DeviceCriteria criteria;
    criteria.setDeviceCountCriteria(1);
    criteria.setTypeCriteria(oul::DEVICE_TYPE_GPU);
    oul::OpenCLManager * manager = oul::OpenCLManager::getInstance();
    oul::Context context = manager->createContext(criteria);

    ImageImporter2D * test = new ImageImporter2D(context, "lena.jpg");
    ImageImporter2DPtr importer = ImageImporter2DPtr(test);
    Image2DPtr image = importer->getOutput();
}
