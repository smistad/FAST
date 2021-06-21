#include "RegionProperties.hpp"
#include <FAST/Testing.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>

using namespace fast;

TEST_CASE("Region properties", "[regionproperties][fast]") {

    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_0.mhd");

    auto segmentation = BinaryThresholding::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->setLowerThreshold(100);

    auto regionProperties = RegionProperties::New();
    regionProperties->setInputConnection(segmentation->getOutputPort());
    auto regionList = regionProperties->updateAndGetOutputData<RegionList>();
    auto regions = regionList->get();

    // TODO validate
    for(auto& region : regions) {
        //std::cout << "Area: " << region.area << std::endl;
        //std::cout << "Label: " << (int)region.label << std::endl;
    }
}