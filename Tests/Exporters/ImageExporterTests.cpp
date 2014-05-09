#include "catch.hpp"
#include "ImageExporter.hpp"
#include "Image.hpp"

using namespace fast;

TEST_CASE("No filename given to the ImageExporter", "[fast][ImageExporter]") {
    Image::pointer image = Image::New();
ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setInput(image);
    CHECK_THROWS(exporter->update());
}

TEST_CASE("No input given to the ImageExporter", "[fast][ImageExporter]") {
    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename("asd");
    CHECK_THROWS(exporter->update());
}

