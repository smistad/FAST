#include "catch.hpp"
#include "ImageImporter2D.hpp"
#include "DeviceManager.hpp"

using namespace fast;

TEST_CASE("No filename set to ImageImporter", "[fast][ImageImporter]") {
    ImageImporter2D::pointer importer = ImageImporter2D::New();
    CHECK_THROWS(importer->update());
}

TEST_CASE("Import non-existing file to ImageImporter", "[fast][ImageImporter]") {
    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename("asdasdasdsad");
    CHECK_THROWS_AS(importer->update(), FileNotFoundException);
}

TEST_CASE("Import Image file to host", "[fast][ImageImporter]") {
    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "lena.jpg");
    importer->setDevice(Host::New());
    Image::pointer image = importer->getOutput();
    image->update();

    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getDataType() == TYPE_FLOAT);
}

TEST_CASE("Import Image file to OpenCL device", "[fast][MetaImageImporter]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "lena.jpg");
    importer->setDevice(device);
    Image::pointer image = importer->getOutput();
    image->update();

    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getDataType() == TYPE_FLOAT);
}

