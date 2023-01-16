#include <FAST/Testing.hpp>
#include "ColorToGrayscale.hpp"
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Data/Image.hpp>

using namespace fast;

TEST_CASE("Color to grayscale", "[fast][ColorToGrayscale]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");
    auto input = importer->runAndGetOutputData<Image>();

    {
        auto converter = ColorToGrayscale::create()->connect(input);
        auto output = converter->runAndGetOutputData<Image>();
        REQUIRE(output->getNrOfChannels() == 1);
        REQUIRE(output->getWidth() == input->getWidth());
        REQUIRE(output->getHeight() == input->getHeight());
        REQUIRE(output->getDataType() == input->getDataType());
    }
}
