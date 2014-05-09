#include "catch.hpp"
#include "MetaImageExporter.hpp"
#include "Image.hpp"

using namespace fast;

TEST_CASE("No filename given to the MetaImageExporter", "[fast][MetaImageExporter]") {
    Image::pointer image = Image::New();
    MetaImageExporter::pointer exporter = MetaImageExporter::New();
    exporter->setInput(image);
    CHECK_THROWS(exporter->update());
}

TEST_CASE("No input given to the MetaImageExporter", "[fast][MetaImageExporter]") {
    MetaImageExporter::pointer exporter = MetaImageExporter::New();
    exporter->setFilename("asd");
    CHECK_THROWS(exporter->update());
}
