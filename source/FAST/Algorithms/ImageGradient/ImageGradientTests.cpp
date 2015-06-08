#include "FAST/Tests/catch.hpp"
#include "FAST/Algorithms/ImageGradient/ImageGradient.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"

using namespace fast;

TEST_CASE("Run ImageGradient on 2D image and OpenCL device", "[fast][ImageGradient]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2Dt/US-2Dt_0.mhd");

    ImageGradient::pointer gradientFilter = ImageGradient::New();
    gradientFilter->setInputConnection(importer->getOutputPort());
    CHECK_NOTHROW(gradientFilter->update());
}
