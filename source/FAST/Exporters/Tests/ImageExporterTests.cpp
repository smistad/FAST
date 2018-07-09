#include "FAST/Testing.hpp"
#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/Importers/ImageImporter.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Tests/DataComparison.hpp"

using namespace fast;

TEST_CASE("No filename given to the ImageExporter", "[fast][ImageExporter]") {
    Image::pointer image = Image::New();
    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setInputData(image);
    CHECK_THROWS(exporter->update(0));
}

TEST_CASE("No input given to the ImageExporter", "[fast][ImageExporter]") {
    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename("asd");
    CHECK_THROWS(exporter->update(0));
}


TEST_CASE("If 3D image is given as input to ImageExporter it throws exception", "[fast][ImageExporter]") {
    Image::pointer image = Image::New();
    image->create(16,16,16,TYPE_INT8,1);
    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename("asd");
    exporter->setInputData(image);
    CHECK_THROWS(exporter->update(0));
}

TEST_CASE("Write 2D image with the ImageExporter", "[fast][ImageExporter]") {

    unsigned int width = 32;
    unsigned int height = 46;
    unsigned int channels = 1;
    DataType type = TYPE_UINT8;

    Image::pointer image = Image::New();
    image->create(width,height,type,channels);

    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename("ImageExporterTest.jpg");
    exporter->setInputData(image);
    exporter->update(0);

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename("ImageExporterTest.jpg");
    auto port = importer->getOutputPort();
    importer->update(0);
    Image::pointer image2 = port->getNextFrame<Image>();

    CHECK(width == image2->getWidth());
    CHECK(height == image2->getHeight());

}

