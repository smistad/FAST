#include "catch.hpp"
#include "Image.hpp"

using namespace fast;

TEST_CASE("Create a 2D image on host", "[fast][image]") {
    Image::pointer image = Image::New();

    unsigned int width = 256;
    unsigned int height = 512;
    unsigned int nrOfComponents = 1;
    DataType type = TYPE_FLOAT;
    image->create2DImage(width, height, type, nrOfComponents, Host::New());

    CHECK(image->getWidth() == width);
    CHECK(image->getHeight() == height);
    CHECK(image->getDepth() == 1);
    CHECK(image->getNrOfComponents() == nrOfComponents);
    CHECK(image->getDataType() == type);
    CHECK(image->getDimensions() == 2);
}

TEST_CASE("Create a 3D image on host", "[fast][image]") {
    Image::pointer image = Image::New();

    unsigned int width = 256;
    unsigned int height = 512;
    unsigned int depth = 45;
    unsigned int nrOfComponents = 2;
    DataType type = TYPE_INT8;
    image->create3DImage(width, height, depth, type, nrOfComponents, Host::New());

    CHECK(image->getWidth() == width);
    CHECK(image->getHeight() == height);
    CHECK(image->getDepth() == depth);
    CHECK(image->getNrOfComponents() == nrOfComponents);
    CHECK(image->getDataType() == type);
    CHECK(image->getDimensions() == 3);
}

TEST_CASE("Create a image twice", "[fast][image]") {
    Image::pointer image = Image::New();

    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New());
    CHECK_THROWS(image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New()));
}
