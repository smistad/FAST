#include "catch.hpp"
#include "ImageExporter2D.hpp"
#include "Image.hpp"

using namespace fast;

TEST_CASE("No filename given to the ImageExporter", "[fast][ImageExporter]") {
    Image::pointer image = Image::New();
ImageExporter2D::pointer exporter = ImageExporter2D::New();
    exporter->setInput(image);
    CHECK_THROWS(exporter->update());
}

TEST_CASE("No input given to the ImageExporter", "[fast][ImageExporter]") {
    ImageExporter2D::pointer exporter = ImageExporter2D::New();
    exporter->setFilename("asd");
    CHECK_THROWS(exporter->update());
}

