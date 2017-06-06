#include "FAST/Testing.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"

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
    importer->setFilename(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_0.mhd");
    importer->setMainDevice(Host::getInstance());
    importer->update();
    Image::pointer image = importer->getOutputData<Image>(0);
    AffineTransformation::pointer T = image->getSceneGraphNode()->getTransformation();

    // Check attributes of image
    CHECK(image->getWidth() == 492);
    CHECK(image->getHeight() == 380);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getSpacing().x() == Approx(0.079));
    CHECK(image->getSpacing().y() == Approx(0.079));
    CHECK(image->getSpacing().z() == Approx(1.0));
    CHECK(T->getTransform().translation().x() == Approx(182.658));
    CHECK(T->getTransform().translation().y() == Approx(-42.554));
    CHECK(T->getTransform().translation().z() == Approx(116.602));
    CHECK(T->getTransform().linear()(0,0) == Approx(0.247264));
    CHECK(T->getTransform().linear()(1,0) == Approx(0.836318));
    CHECK(T->getTransform().linear()(2,0) == Approx(0.489349));
    CHECK(T->getTransform().linear()(0,1) == Approx(0.816965));
    CHECK(T->getTransform().linear()(1,1) == Approx(-0.451445));
    CHECK(T->getTransform().linear()(2,1) == Approx(0.358714));
    CHECK(T->getTransform().linear()(0,2) == Approx(-0.520911));
    CHECK(T->getTransform().linear()(1,2) == Approx(-0.311053));
    CHECK(T->getTransform().linear()(2,2) == Approx(0.794878));
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import 3D MetaImage file to host", "[fast][MetaImageImporter]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/Ball/US-3Dt_0.mhd");
    importer->setMainDevice(Host::getInstance());
    importer->update();
    Image::pointer image = importer->getOutputData<Image>(0);
    AffineTransformation::pointer T = image->getSceneGraphNode()->getTransformation();

    // Check attributes of image
    CHECK(image->getWidth() == 276);
    CHECK(image->getHeight() == 249);
    CHECK(image->getDepth() == 200);
    CHECK(image->getDimensions() == 3);
    CHECK(image->getSpacing().x() == Approx(0.309894));
    CHECK(image->getSpacing().y() == Approx(0.241966));
    CHECK(image->getSpacing().z() == Approx(0.430351));
    CHECK(T->getTransform().translation().x() == Approx(-20.2471));
    CHECK(T->getTransform().translation().y() == Approx(-191.238));
    CHECK(T->getTransform().translation().z() == Approx(-65.9711));
    CHECK(T->getTransform().linear()(0,0) == Approx(0.0784201));
    CHECK(T->getTransform().linear()(1,0) == Approx(0.0356554));
    CHECK(T->getTransform().linear()(2,0) == Approx(-0.996283));
    CHECK(T->getTransform().linear()(0,1) == Approx(-0.0697932));
    CHECK(T->getTransform().linear()(1,1) == Approx(0.997105));
    CHECK(T->getTransform().linear()(2,1) == Approx(0.0301913));
    CHECK(T->getTransform().linear()(0,2) == Approx(0.994474));
    CHECK(T->getTransform().linear()(1,2) == Approx(0.0671661));
    CHECK(T->getTransform().linear()(2,2) == Approx(0.0806815));
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import MetaImage file to OpenCL device", "[fast][MetaImageImporter]") {
    DeviceManager* deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager->getOneOpenCLDevice();

    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/Ball/US-3Dt_0.mhd");
    importer->setMainDevice(device);
    importer->update();
    Image::pointer image = importer->getOutputData<Image>(0);
    AffineTransformation::pointer T = image->getSceneGraphNode()->getTransformation();

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

