#include "catch.hpp"
#include "SeededRegionGrowing.hpp"
#include "ImageImporter.hpp"
#include "MetaImageImporter.hpp"
#include "ImageRenderer.hpp"
#include "SimpleWindow.hpp"
#include "DeviceManager.hpp"

namespace fast {

TEST_CASE("2D Seeded region growing on OpenCL device", "[fast][SeededRegionGrowing]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2D.jpg");
    
    SeededRegionGrowing::pointer algorithm = SeededRegionGrowing::New();
    algorithm->setInput(importer->getOutput());
    algorithm->addSeedPoint(45,248);
    algorithm->addSeedPoint(321,181);
    algorithm->setIntensityRange(0.1,1.0);
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

TEST_CASE("3D Seeded region growing on OpenCL device", "[fast][SeededRegionGrowing]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_0.mhd");

    SeededRegionGrowing::pointer algorithm = SeededRegionGrowing::New();
    algorithm->setInput(importer->getOutput());
    algorithm->addSeedPoint(100,100,100);
    algorithm->setIntensityRange(50, 255);
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
    CHECK(4107760 == sum);
}

} // end namespace fast
