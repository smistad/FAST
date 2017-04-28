#include "FAST/Testing.hpp"
#include "SeededRegionGrowing.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Segmentation.hpp"

namespace fast {

TEST_CASE("2D Seeded region growing on OpenCL device", "[fast][SeededRegionGrowing]") {
    std::vector<OpenCLDevice::pointer> devices = DeviceManager::getInstance()->getAllDevices();
    for(int i = 0; i < devices.size(); i++) {
        INFO("Device " << devices[i]->getName());
        ImageFileImporter::pointer importer = ImageFileImporter::New();
        importer->setFilename(Config::getTestDataPath()+"US/Heart/ApicalFourChamber/US-2D_0.mhd");
        importer->setMainDevice(devices[i]);

        SeededRegionGrowing::pointer algorithm = SeededRegionGrowing::New();
        algorithm->setInputConnection(importer->getOutputPort());
        algorithm->addSeedPoint(50,50);
        algorithm->addSeedPoint(100,100);
        algorithm->setIntensityRange(26,255);
        algorithm->setMainDevice(devices[i]);
        algorithm->update();
        Segmentation::pointer result = algorithm->getOutputData<Segmentation>();

        // Temporary check of how many pixels where segmented
        // Should be replaced by result matching
        ImageAccess::pointer access = result->getImageAccess(ACCESS_READ);
        uchar* data = (uchar*)access->get();
        int sum = 0;
        for(int i = 0; i < result->getWidth()*result->getHeight(); i++) {
            if(data[i] == 1)
                sum++;
        }
        CHECK(72640 == sum);
    }
}



TEST_CASE("3D Seeded region growing on OpenCL device", "[fast][SeededRegionGrowing]") {
    std::vector<OpenCLDevice::pointer> devices = DeviceManager::getInstance()->getAllDevices();
    for(int i = 0; i < devices.size(); i++) {
        INFO("Device " << devices[i]->getName());
        ImageFileImporter::pointer importer = ImageFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");
        importer->setMainDevice(devices[i]);

        SeededRegionGrowing::pointer algorithm = SeededRegionGrowing::New();
        algorithm->setInputConnection(importer->getOutputPort());
        algorithm->addSeedPoint(100,100,100);
        algorithm->setIntensityRange(50, 255);
        algorithm->setMainDevice(devices[i]);
        algorithm->update();
        Segmentation::pointer result = algorithm->getOutputData<Segmentation>();

        // Temporary check of how many pixels where segmented
        // Should be replaced by result matching
        ImageAccess::pointer access = result->getImageAccess(ACCESS_READ);
        uchar* data = (uchar*)access->get();
        int sum = 0;
        for(int i = 0; i < result->getWidth()*result->getHeight()*result->getDepth(); i++) {
            if(data[i] == 1)
                sum++;
        }
        CHECK(4106484 == sum);
    }
}

TEST_CASE("3D Seeded region growing on Host", "[fast][SeededRegionGrowing]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");

    SeededRegionGrowing::pointer algorithm = SeededRegionGrowing::New();
    algorithm->setInputConnection(importer->getOutputPort());
    algorithm->addSeedPoint(100,100,100);
    algorithm->setIntensityRange(50, 255);
    algorithm->setMainDevice(Host::getInstance());
    algorithm->update();
    Segmentation::pointer result = algorithm->getOutputData<Segmentation>();

    // Temporary check of how many pixels where segmented
    // Should be replaced by result matching
    ImageAccess::pointer access = result->getImageAccess(ACCESS_READ);
    uchar* data = (uchar*)access->get();
    int sum = 0;
    for(int i = 0; i < result->getWidth()*result->getHeight()*result->getDepth(); i++) {
        if(data[i] == 1)
            sum++;
    }
    CHECK(4106484 == sum);
}

} // end namespace fast
