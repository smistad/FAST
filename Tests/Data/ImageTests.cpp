#include "catch.hpp"
#include "Image.hpp"
#include <ctime>
#include <cmath>

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

TEST_CASE("Create a 2D image on host with input data", "[fast][image]") {

    Image::pointer image = Image::New();

    unsigned int width = 256;
    unsigned int height = 512;

    // Create a data array with random data
    uchar* data = new uchar[width*height];
    srand(time(NULL));
    for(unsigned int i = 0; i < width*height; i++) {
        data[i] = rand() % 255;
    }

    image->create2DImage(width, height, TYPE_UINT8, 1, Host::New(), data);

    ImageAccess access = image->getImageAccess(ACCESS_READ);
    uchar* returnData = (uchar*)access.get();

    bool success = true;
    for(unsigned int i = 0; i < width*height; i++) {
        if(data[i] != returnData[i]) {
            success = false;
            break;
        }
    }

    CHECK(success == true);

    delete[] data;
}

TEST_CASE("Create a 3D image on host with input data", "[fast][image]") {

    Image::pointer image = Image::New();

    unsigned int width = 256;
    unsigned int height = 512;
    unsigned int depth = 45;

    // Create a data array with random data
    uchar* data = new uchar[width*height*depth];
    srand(time(NULL));
    for(unsigned int i = 0; i < width*height*depth; i++) {
        data[i] = rand() % 255;
    }

    image->create3DImage(width, height, depth, TYPE_UINT8, 1, Host::New(), data);

    ImageAccess access = image->getImageAccess(ACCESS_READ);
    uchar* returnData = (uchar*)access.get();

    bool success = true;
    for(unsigned int i = 0; i < width*height*depth; i++) {
        if(data[i] != returnData[i]) {
            success = false;
            break;
        }
    }

    CHECK(success == true);

    delete[] data;
}

TEST_CASE("Create an image twice", "[fast][image]") {
    Image::pointer image = Image::New();

    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New());
    CHECK_THROWS(image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New()));
}
