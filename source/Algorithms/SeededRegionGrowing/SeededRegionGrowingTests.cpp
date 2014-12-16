#include "catch.hpp"
#include "SeededRegionGrowing.hpp"
#include "ImageImporter.hpp"
#include "MetaImageImporter.hpp"
#include "ImageRenderer.hpp"
#include "SimpleWindow.hpp"
#include "DeviceManager.hpp"

namespace fast {

TEST_CASE("2D Seeded region growing on OpenCL device", "[fast][SeededRegionGrowing]") {
    std::vector<OpenCLDevice::pointer> devices = DeviceManager::getInstance().getAllDevices();
    for(int i = 0; i < devices.size(); i++) {
        INFO("Device " << devices[i]->getName());
        ImageImporter::pointer importer = ImageImporter::New();
        importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2D.jpg");
        importer->setMainDevice(devices[i]);

        SeededRegionGrowing::pointer algorithm = SeededRegionGrowing::New();
        algorithm->setInput(importer->getOutput());
        algorithm->addSeedPoint(45,248);
        algorithm->addSeedPoint(321,181);
        algorithm->setIntensityRange(0.1,1.0);
        algorithm->setMainDevice(devices[i]);
        Image::pointer result = algorithm->getOutput();
        result->update();

        // Temporary check of how many pixels where segmented
        // Should be replaced by result matching
        ImageAccess access = result->getImageAccess(ACCESS_READ);
        uchar* data = (uchar*)access.get();
        int sum = 0;
        for(int i = 0; i < result->getWidth()*result->getHeight(); i++) {
            if(data[i] == 1)
                sum++;
        }
        CHECK(17893 == sum);
    }
}



TEST_CASE("3D Seeded region growing on OpenCL device", "[fast][SeededRegionGrowing]") {
    std::vector<OpenCLDevice::pointer> devices = DeviceManager::getInstance().getAllDevices();
    for(int i = 0; i < devices.size(); i++) {
        INFO("Device " << devices[i]->getName());
        MetaImageImporter::pointer importer = MetaImageImporter::New();
        importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_0.mhd");
        importer->setMainDevice(devices[i]);

        SeededRegionGrowing::pointer algorithm = SeededRegionGrowing::New();
        algorithm->setInput(importer->getOutput());
        algorithm->addSeedPoint(100,100,100);
        algorithm->setIntensityRange(50, 255);
        algorithm->setMainDevice(devices[i]);
        Image::pointer result = algorithm->getOutput();
        result->update();

        // Temporary check of how many pixels where segmented
        // Should be replaced by result matching
        ImageAccess access = result->getImageAccess(ACCESS_READ);
        uchar* data = (uchar*)access.get();
        int sum = 0;
        for(int i = 0; i < result->getWidth()*result->getHeight()*result->getDepth(); i++) {
            if(data[i] == 1)
                sum++;
        }
        CHECK(4106484 == sum);
    }
}

TEST_CASE("3D Seeded region growing on Host", "[fast][SeededRegionGrowing]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_0.mhd");

    SeededRegionGrowing::pointer algorithm = SeededRegionGrowing::New();
    algorithm->setInput(importer->getOutput());
    algorithm->addSeedPoint(100,100,100);
    algorithm->setIntensityRange(50, 255);
    algorithm->setMainDevice(Host::getInstance());
    Image::pointer result = algorithm->getOutput();
    result->update();

    // Temporary check of how many pixels where segmented
    // Should be replaced by result matching
    ImageAccess access = result->getImageAccess(ACCESS_READ);
    uchar* data = (uchar*)access.get();
    int sum = 0;
    for(int i = 0; i < result->getWidth()*result->getHeight()*result->getDepth(); i++) {
        if(data[i] == 1)
            sum++;
    }
    CHECK(4106484 == sum);
}

} // end namespace fast
