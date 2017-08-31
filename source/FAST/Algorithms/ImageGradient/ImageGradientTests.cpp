#include "FAST/Testing.hpp"
#include "FAST/Algorithms/ImageGradient/ImageGradient.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"

using namespace fast;

TEST_CASE("Run ImageGradient on 2D image and OpenCL device", "[fast][ImageGradient]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_0.mhd");

    ImageGradient::pointer gradientFilter = ImageGradient::New();
    gradientFilter->setInputConnection(importer->getOutputPort());
    CHECK_NOTHROW(gradientFilter->update(0));
}

TEST_CASE("Run ImageGradient on 2D image and OpenCL device 16 bit", "[fast][ImageGradient]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_0.mhd");

    ImageGradient::pointer gradientFilter = ImageGradient::New();
    gradientFilter->set16bitStorageFormat();
    gradientFilter->setInputConnection(importer->getOutputPort());
    CHECK_NOTHROW(gradientFilter->update(0));
}
