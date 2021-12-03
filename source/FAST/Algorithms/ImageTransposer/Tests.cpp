#include <FAST/Testing.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include "ImageTransposer.hpp"
#include <FAST/Data/Image.hpp>

using namespace fast;

TEST_CASE("Image transposer 2D", "[fast][ImageTransposer]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");
    auto image = importer->runAndGetOutputData<Image>();

    auto transposer = ImageTransposer::create()->connect(image);
    auto transposedImage = transposer->runAndGetOutputData<Image>();

    CHECK(image->getWidth() == transposedImage->getHeight());
    CHECK(image->getHeight() == transposedImage->getWidth());
    CHECK(image->getNrOfChannels() == transposedImage->getNrOfChannels());
    CHECK(image->getDataType() == transposedImage->getDataType());
    CHECK(image->getSpacing().x() == transposedImage->getSpacing().y());
    CHECK(image->getSpacing().y() == transposedImage->getSpacing().x());
}

TEST_CASE("Image transposer 3D", "[fast][ImageTransposer]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");
    auto image = importer->runAndGetOutputData<Image>();

    {
        auto transposer = ImageTransposer::create()->connect(image);
        auto transposedImage = transposer->runAndGetOutputData<Image>();

        CHECK(image->getWidth() == transposedImage->getHeight());
        CHECK(image->getHeight() == transposedImage->getWidth());
        CHECK(image->getDepth() == transposedImage->getDepth());
        CHECK(image->getNrOfChannels() == transposedImage->getNrOfChannels());
        CHECK(image->getDataType() == transposedImage->getDataType());
        CHECK(image->getSpacing().x() == transposedImage->getSpacing().y());
        CHECK(image->getSpacing().y() == transposedImage->getSpacing().x());
        CHECK(image->getSpacing().z() == transposedImage->getSpacing().z());
    }

    {
        auto transposer = ImageTransposer::create({0, 2, 1})->connect(image);
        auto transposedImage = transposer->runAndGetOutputData<Image>();

        CHECK(image->getWidth() == transposedImage->getWidth());
        CHECK(image->getHeight() == transposedImage->getDepth());
        CHECK(image->getDepth() == transposedImage->getHeight());
        CHECK(image->getNrOfChannels() == transposedImage->getNrOfChannels());
        CHECK(image->getDataType() == transposedImage->getDataType());
        CHECK(image->getSpacing().x() == transposedImage->getSpacing().x());
        CHECK(image->getSpacing().y() == transposedImage->getSpacing().z());
        CHECK(image->getSpacing().z() == transposedImage->getSpacing().y());
    }

    {
        auto transposer = ImageTransposer::create({2, 1, 0})->connect(image);
        auto transposedImage = transposer->runAndGetOutputData<Image>();

        CHECK(image->getWidth() == transposedImage->getDepth());
        CHECK(image->getHeight() == transposedImage->getHeight());
        CHECK(image->getDepth() == transposedImage->getWidth());
        CHECK(image->getNrOfChannels() == transposedImage->getNrOfChannels());
        CHECK(image->getDataType() == transposedImage->getDataType());
        CHECK(image->getSpacing().x() == transposedImage->getSpacing().z());
        CHECK(image->getSpacing().y() == transposedImage->getSpacing().y());
        CHECK(image->getSpacing().z() == transposedImage->getSpacing().x());
    }
}
