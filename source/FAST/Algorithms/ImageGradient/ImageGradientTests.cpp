#include "FAST/Testing.hpp"
#include "FAST/Algorithms/ImageGradient/ImageGradient.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"

using namespace fast;

TEST_CASE("Run ImageGradient on 2D image and OpenCL device", "[fast][ImageGradient]") {
    auto importer = MetaImageImporter::create(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_0.mhd");
    auto gradientFilter = ImageGradient::create()->connect(importer);
    CHECK_NOTHROW(gradientFilter->update());
}

TEST_CASE("Run ImageGradient on 2D image and OpenCL device 16 bit", "[fast][ImageGradient]") {
    auto importer = MetaImageImporter::create(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_0.mhd");
    auto gradientFilter = ImageGradient::create(true)->connect(importer);
    CHECK_NOTHROW(gradientFilter->update());
}
