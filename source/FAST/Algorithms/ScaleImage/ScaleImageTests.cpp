#include "FAST/Testing.hpp"
#include "ScaleImage.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

TEST_CASE("Scale image 2D", "[fast][ScaleImage]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2Dt/US-2Dt_0.mhd");

    ScaleImage::pointer normalize = ScaleImage::New();
    normalize->setInputConnection(importer->getOutputPort());
    normalize->update();

    Image::pointer result = normalize->getOutputData<Image>();

    CHECK(result->calculateMinimumIntensity() == Approx(0));
    CHECK(result->calculateMaximumIntensity() == Approx(1));
}

TEST_CASE("Scale image 3D", "[fast][ScaleImage]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "CT-Abdomen.mhd");

    ScaleImage::pointer normalize = ScaleImage::New();
    normalize->setInputConnection(importer->getOutputPort());
    normalize->update();

    Image::pointer result = normalize->getOutputData<Image>();

    CHECK(result->calculateMinimumIntensity() == Approx(0));
    CHECK(result->calculateMaximumIntensity() == Approx(1));
}

TEST_CASE("Scale image 2D with high and low set", "[fast][ScaleImage]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2Dt/US-2Dt_0.mhd");

    ScaleImage::pointer normalize = ScaleImage::New();
    normalize->setInputConnection(importer->getOutputPort());
    normalize->setLowestValue(-2);
    normalize->setHighestValue(10);
    normalize->update();

    Image::pointer result = normalize->getOutputData<Image>();

    CHECK(result->calculateMinimumIntensity() == Approx(-2));
    CHECK(result->calculateMaximumIntensity() == Approx(10));
}

TEST_CASE("Scale image 3D with high and low set", "[fast][ScaleImage]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "CT-Abdomen.mhd");

    ScaleImage::pointer normalize = ScaleImage::New();
    normalize->setInputConnection(importer->getOutputPort());
    normalize->setLowestValue(-2);
    normalize->setHighestValue(10);
    normalize->update();

    Image::pointer result = normalize->getOutputData<Image>();

    CHECK(result->calculateMinimumIntensity() == Approx(-2));
    CHECK(result->calculateMaximumIntensity() == Approx(10));
}


}
