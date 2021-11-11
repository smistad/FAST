#include <FAST/Testing.hpp>
#include "ImagePyramidLevelExtractor.hpp"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>

using namespace fast;

TEST_CASE("Image pyramid level extractor", "[fast][ImagePyramidLevelExtractor]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/A05.svs");
    auto pyramid = importer->runAndGetOutputData<ImagePyramid>();

    auto extractor = ImagePyramidLevelExtractor::create(-1)->connect(pyramid);
    auto level = extractor->runAndGetOutputData<Image>();

    CHECK(pyramid->getLevelWidth(-1) == level->getWidth());
    CHECK(pyramid->getLevelHeight(-1) == level->getHeight());
}