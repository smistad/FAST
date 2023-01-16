#include <FAST/Testing.hpp>
#include "GrayscaleToColor.hpp"
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Data/Image.hpp>

using namespace fast;

TEST_CASE("Grayscale to color", "[fast][GrayscaleToColor]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");
    auto input = importer->runAndGetOutputData<Image>();

    {
        auto converter = GrayscaleToColor::create()->connect(input);
        auto output = converter->runAndGetOutputData<Image>();
        REQUIRE(output->getNrOfChannels() == 3);
        REQUIRE(output->getWidth() == input->getWidth());
        REQUIRE(output->getHeight() == input->getHeight());
        REQUIRE(output->getDataType() == input->getDataType());
    }
    {
        auto converter = GrayscaleToColor::create(true)->connect(input);
        auto output = converter->runAndGetOutputData<Image>();
        REQUIRE(output->getNrOfChannels() == 4);
        REQUIRE(output->getWidth() == input->getWidth());
        REQUIRE(output->getHeight() == input->getHeight());
        REQUIRE(output->getDataType() == input->getDataType());
    }
}