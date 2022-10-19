#include <FAST/Testing.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include "ImageFlipper.hpp"
#include <FAST/Data/Image.hpp>

using namespace fast;

TEST_CASE("Image flip 2D", "[fast][ImageFlipper]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");
    auto image = importer->runAndGetOutputData<Image>();

    auto flipper = ImageFlipper::create(true, false)->connect(image);
    auto flippedImage = flipper->runAndGetOutputData<Image>();

    CHECK(image->getWidth() == flippedImage->getWidth());
    CHECK(image->getHeight() == flippedImage->getHeight());
    CHECK(image->getNrOfChannels() == flippedImage->getNrOfChannels());
    CHECK(image->getDataType() == flippedImage->getDataType());
    CHECK(image->getSpacing().x() == flippedImage->getSpacing().x());
    CHECK(image->getSpacing().y() == flippedImage->getSpacing().y());

    // TODO check pixels
}

TEST_CASE("Image flip 3D", "[fast][ImageFlipper]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");
    auto image = importer->runAndGetOutputData<Image>();

    {
        auto flipper = ImageFlipper::create(true, false, false)->connect(image);
        auto flippedImage = flipper->runAndGetOutputData<Image>();

	    CHECK(image->getWidth() == flippedImage->getWidth());
	    CHECK(image->getHeight() == flippedImage->getHeight());
	    CHECK(image->getDepth() == flippedImage->getDepth());
	    CHECK(image->getNrOfChannels() == flippedImage->getNrOfChannels());
	    CHECK(image->getDataType() == flippedImage->getDataType());
	    CHECK(image->getSpacing().x() == flippedImage->getSpacing().x());
	    CHECK(image->getSpacing().y() == flippedImage->getSpacing().y());
	    CHECK(image->getSpacing().z() == flippedImage->getSpacing().z());
    }

    // TODO check pixels
}
