#include "FAST/Testing.hpp"
#include "FAST/Importers/ImageImporter.hpp"
#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

TEST_CASE("No filename set to ImageImporter", "[fast][ImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    CHECK_THROWS(importer->update());
}

TEST_CASE("Import non-existing file to ImageImporter", "[fast][ImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename("asdasdasdsad.bmp");
    CHECK_THROWS_AS(importer->update(), FileNotFoundException);
}

#ifdef FAST_MODULE_VISUALIZATION

TEST_CASE("Import PNG image file to host", "[fast][ImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/US-2D.png");
    importer->setMainDevice(Host::getInstance());
    DataChannel::pointer port = importer->getOutputPort();
    importer->update();
    Image::pointer image = port->getNextFrame<Image>();


    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import PNG image file to OpenCL device", "[fast][ImageImporter]") {
    DeviceManager* deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager->getOneOpenCLDevice();

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/US-2D.png");
    importer->setMainDevice(device);
    DataChannel::pointer port = importer->getOutputPort();
    importer->update();
    Image::pointer image = port->getNextFrame<Image>();

    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getNrOfChannels() == 3);
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import grayscale PNG image file to OpenCL device", "[fast][ImageImporter]") {
    DeviceManager* deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager->getOneOpenCLDevice();

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setGrayscale(true);
    importer->setFilename(Config::getTestDataPath() + "US/US-2D.png");
    importer->setMainDevice(device);
    DataChannel::pointer port = importer->getOutputPort();
    importer->update();
    Image::pointer image = port->getNextFrame<Image>();

    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getNrOfChannels() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import JPEG image file to host", "[fast][ImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/US-2D.jpg");
    importer->setMainDevice(Host::getInstance());
    DataChannel::pointer port = importer->getOutputPort();
    importer->update();
    Image::pointer image = port->getNextFrame<Image>();

    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import JPEG image file to OpenCL device", "[fast][ImageImporter]") {
    DeviceManager* deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager->getOneOpenCLDevice();

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/US-2D.jpg");
    importer->setMainDevice(device);
    DataChannel::pointer port = importer->getOutputPort();
    importer->update();
    Image::pointer image = port->getNextFrame<Image>();

    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getDataType() == TYPE_UINT8);
}

#endif

TEST_CASE("Import BMP image file to host", "[fast][ImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/US-2D.bmp");
    importer->setMainDevice(Host::getInstance());
    DataChannel::pointer port = importer->getOutputPort();
    importer->update();
    Image::pointer image = port->getNextFrame<Image>();

    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import BMP image file to OpenCL device", "[fast][ImageImporter]") {
    DeviceManager* deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager->getOneOpenCLDevice();

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/US-2D.bmp");
    importer->setMainDevice(device);
    DataChannel::pointer port = importer->getOutputPort();
    importer->update();
    Image::pointer image = port->getNextFrame<Image>();

    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import JPEG XL image", "[fast][ImageImporter][JPEGXL]") {
    auto importer = ImageImporter::create(Config::getTestDataPath() + "/US/US-2D.jpg");

    auto exporter = ImageExporter::create("ImageExporterTest.jxl")
            ->connect(importer);
    exporter->run();

    auto importer2 = ImageImporter::create("ImageExporterTest.jxl");
    auto image = importer2->runAndGetOutputData<Image>();

    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getNrOfChannels() == 3);
    CHECK(image->getDataType() == TYPE_UINT8);

    //auto renderer = ImageRenderer::create()->connect(image);
    //SimpleWindow2D::create()->connect(renderer)->run();
}

