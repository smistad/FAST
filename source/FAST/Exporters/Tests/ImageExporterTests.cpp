#include "FAST/Testing.hpp"
#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/Importers/ImageImporter.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Tests/DataComparison.hpp"

using namespace fast;

TEST_CASE("No filename given to the ImageExporter", "[fast][ImageExporter]") {
    Image::pointer image = Image::create(32, 32, TYPE_FLOAT, 1);
    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setInputData(image);
    CHECK_THROWS(exporter->update());
}

TEST_CASE("No input given to the ImageExporter", "[fast][ImageExporter]") {
    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename("asd");
    CHECK_THROWS(exporter->update());
}


TEST_CASE("If 3D image is given as input to ImageExporter it throws exception", "[fast][ImageExporter]") {
    Image::pointer image = Image::create(16,16,16,TYPE_INT8,1);
    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename("asd");
    exporter->setInputData(image);
    CHECK_THROWS(exporter->update());
}

TEST_CASE("Write 2D image with the ImageExporter", "[fast][ImageExporter]") {
    unsigned int width = 32;
    unsigned int height = 46;
    unsigned int channels = 2;
    DataType type = TYPE_UINT8;

    Image::pointer image = Image::create(width,height,type,channels);
    image->fill(0);

    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename("ImageExporterTest.jpg");
    exporter->setInputData(image);
    exporter->update();

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename("ImageExporterTest.jpg");
    auto port = importer->getOutputPort();
    importer->update();
    Image::pointer image2 = port->getNextFrame<Image>();

    CHECK(width == image2->getWidth());
    CHECK(height == image2->getHeight());
}


TEST_CASE("Write anisotropic 2D image with the ImageExporter", "[fast][ImageExporter]") {
    unsigned int width = 32;
    unsigned int height = 46;
    unsigned int channels = 3;
    DataType type = TYPE_UINT8;

    auto image = Image::create(width,height,type,channels);
    image->fill(0);
    image->setSpacing(Vector3f(1.0f, 2.0f, 0.0f));

    auto exporter = ImageExporter::create("ImageExporterTest.jpg");
    exporter->setInputData(image);
    exporter->update();

    auto importer = ImageImporter::create("ImageExporterTest.jpg", false);
    auto image2 = importer->runAndGetOutputData<Image>();

    CHECK(channels == image2->getNrOfChannels());
    CHECK(width == image2->getWidth());
    CHECK(height*2 == image2->getHeight());
}
