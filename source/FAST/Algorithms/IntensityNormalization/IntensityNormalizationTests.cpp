#include "FAST/Testing.hpp"
#include "IntensityNormalization.hpp"
#include "ZeroMeanUnitVariance.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

TEST_CASE("IntensityNormalization 2D", "[fast][IntensityNormalization]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_0.mhd");

    IntensityNormalization::pointer normalize = IntensityNormalization::New();
    normalize->setInputConnection(importer->getOutputPort());
    auto port = normalize->getOutputPort();
    normalize->update();

    Image::pointer result = port->getNextFrame<Image>();

    CHECK(result->calculateMinimumIntensity() == Approx(0));
    CHECK(result->calculateMaximumIntensity() == Approx(1));
}

TEST_CASE("IntensityNormalization 3D", "[fast][IntensityNormalization]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CT/CT-Abdomen.mhd");

    IntensityNormalization::pointer normalize = IntensityNormalization::New();
    normalize->setInputConnection(importer->getOutputPort());
    auto port = normalize->getOutputPort();
    normalize->update();

    Image::pointer result = port->getNextFrame<Image>();

    CHECK(result->calculateMinimumIntensity() == Approx(0));
    CHECK(result->calculateMaximumIntensity() == Approx(1));
}

TEST_CASE("IntensityNormalization image 2D with high and low set", "[fast][IntensityNormalization]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_0.mhd");

    IntensityNormalization::pointer normalize = IntensityNormalization::New();
    normalize->setInputConnection(importer->getOutputPort());
    normalize->setLowestValue(-2);
    normalize->setHighestValue(10);
    auto port = normalize->getOutputPort();
    normalize->update();

    Image::pointer result = port->getNextFrame<Image>();

    CHECK(result->calculateMinimumIntensity() == Approx(-2));
    CHECK(result->calculateMaximumIntensity() == Approx(10));
}

TEST_CASE("IntensityNormalization image 3D with high and low set", "[fast][IntensityNormalization]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CT/CT-Abdomen.mhd");

    IntensityNormalization::pointer normalize = IntensityNormalization::New();
    normalize->setInputConnection(importer->getOutputPort());
    normalize->setLowestValue(-2);
    normalize->setHighestValue(10);
    auto port = normalize->getOutputPort();
    normalize->update();

    Image::pointer result = port->getNextFrame<Image>();

    CHECK(result->calculateMinimumIntensity() == Approx(-2));
    CHECK(result->calculateMaximumIntensity() == Approx(10));
}

TEST_CASE("ZeroMeanUnitVariance 2D", "[fast][ZeroMeanUnitVariance]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_0.mhd");

    auto normalize = ZeroMeanUnitVariance::create()->connect(importer);

    auto image = normalize->runAndGetOutputData<Image>();

    CHECK(image->calculateAverageIntensity() == Approx(0).margin(0.01));
    CHECK(image->calculateStandardDeviationIntensity() == Approx(1).margin(0.01));
}

TEST_CASE("ZeroMeanUnitVariance 3D", "[fast][ZeroMeanUnitVariance]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "MRI/MR-Abdomen.mhd");

    auto normalize = ZeroMeanUnitVariance::create()->connect(importer);

    auto image = normalize->runAndGetOutputData<Image>();

    CHECK(image->calculateAverageIntensity() == Approx(0).margin(0.1));
    CHECK(image->calculateStandardDeviationIntensity() == Approx(1).margin(0.1));
}
}
