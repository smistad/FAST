#include "FAST/Tests/catch.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("No filename set to MetaImageImporter", "[fast][MetaImageImporter]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    CHECK_THROWS(importer->update());
}

TEST_CASE("Import non-existing file to MetaImageImporter", "[fast][MetaImageImporter]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename("asdasdasdsad");
    CHECK_THROWS_AS(importer->update(), FileNotFoundException);
}

TEST_CASE("Import 2D MetaImage file to host", "[fast][MetaImageImporter]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2Dt/US-2Dt_0.mhd");
    importer->setMainDevice(Host::getInstance());
    importer->update();
    Image::pointer image = importer->getOutputData<Image>(0);
    AffineTransformation T = image->getSceneGraphNode()->getTransformation();

    // Check attributes of image
    CHECK(image->getWidth() == 471);
    CHECK(image->getHeight() == 616);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getSpacing().x() == Approx(0.081));
    CHECK(image->getSpacing().y() == Approx(0.081));
    CHECK(image->getSpacing().z() == Approx(1.0));
    CHECK(T.translation().x() == Approx(32.2747));
    CHECK(T.translation().y() == Approx(-2.02993));
    CHECK(T.translation().z() == Approx(170.997));
    CHECK(T.linear()(0,0) == Approx(-0.963423));
    CHECK(T.linear()(1,0) == Approx(-0.263617));
    CHECK(T.linear()(2,0) == Approx(-0.0481956));
    CHECK(T.linear()(0,1) == Approx(0.0225832));
    CHECK(T.linear()(1,1) == Approx(0.0993386));
    CHECK(T.linear()(2,1) == Approx(-0.994797));
    CHECK(T.linear()(0,2) == Approx(0.267034));
    CHECK(T.linear()(1,2) == Approx(-0.959499));
    CHECK(T.linear()(2,2) == Approx(-0.089752));
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import 3D MetaImage file to host", "[fast][MetaImageImporter]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_0.mhd");
    importer->setMainDevice(Host::getInstance());
    importer->update();
    Image::pointer image = importer->getOutputData<Image>(0);
    AffineTransformation T = image->getSceneGraphNode()->getTransformation();

    // Check attributes of image
    CHECK(image->getWidth() == 276);
    CHECK(image->getHeight() == 249);
    CHECK(image->getDepth() == 200);
    CHECK(image->getDimensions() == 3);
    CHECK(image->getSpacing().x() == Approx(0.309894));
    CHECK(image->getSpacing().y() == Approx(0.241966));
    CHECK(image->getSpacing().z() == Approx(0.430351));
    CHECK(T.translation().x() == Approx(-20.2471));
    CHECK(T.translation().y() == Approx(-191.238));
    CHECK(T.translation().z() == Approx(-65.9711));
    CHECK(T.linear()(0,0) == Approx(0.0784201));
    CHECK(T.linear()(1,0) == Approx(0.0356554));
    CHECK(T.linear()(2,0) == Approx(-0.996283));
    CHECK(T.linear()(0,1) == Approx(-0.0697932));
    CHECK(T.linear()(1,1) == Approx(0.997105));
    CHECK(T.linear()(2,1) == Approx(0.0301913));
    CHECK(T.linear()(0,2) == Approx(0.994474));
    CHECK(T.linear()(1,2) == Approx(0.0671661));
    CHECK(T.linear()(2,2) == Approx(0.0806815));
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import MetaImage file to OpenCL device", "[fast][MetaImageImporter]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_0.mhd");
    importer->setMainDevice(device);
    importer->update();
    Image::pointer image = importer->getOutputData<Image>(0);
    AffineTransformation T = image->getSceneGraphNode()->getTransformation();

    // Check attributes of image
    CHECK(image->getWidth() == 276);
    CHECK(image->getHeight() == 249);
    CHECK(image->getDepth() == 200);
    CHECK(image->getSpacing().x() == Approx(0.309894));
    CHECK(image->getSpacing().y() == Approx(0.241966));
    CHECK(image->getSpacing().z() == Approx(0.430351));
    CHECK(image->getDimensions() == 3);
    CHECK(image->getDataType() == TYPE_UINT8);
}

/*
TEST_CASE("Import compressed raw file with MetaImage", "[fast][MetaImageImporter][visual]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2D-compressed.mhd");
    importer->update();

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->start();
}
*/
