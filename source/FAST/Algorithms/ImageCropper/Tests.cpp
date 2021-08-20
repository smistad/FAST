#include <FAST/Testing.hpp>
#include "ImageCropper.hpp"
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Data/Image.hpp>

using namespace fast;

TEST_CASE("ImageCropper 2D", "[fast][ImageCropper]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/US-2D.png");
    auto cropper = ImageCropper::create(Vector2i(32, 34), Vector2i(2, 4))->connect(importer);
    cropper->run();
    auto image = cropper->getOutputData<Image>();
    CHECK(image->getWidth() == 32);
    CHECK(image->getHeight() == 34);
}

TEST_CASE("ImageCropper 2D - No offset", "[fast][ImageCropper]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/US-2D.png");
    auto cropper = ImageCropper::create(Vector2i(32, 34))->connect(importer);
    cropper->run();
    auto image = cropper->getOutputData<Image>();
    CHECK(image->getWidth() == 32);
    CHECK(image->getHeight() == 34);
}


TEST_CASE("ImageCropper 2D - Crop Bottom", "[fast][ImageCropper]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/US-2D.png");
    auto cropper = ImageCropper::create(0.25)->connect(importer);
    cropper->run();
    auto image = cropper->getOutputData<Image>();
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 128);
}


TEST_CASE("ImageCropper 2D - Crop Top", "[fast][ImageCropper]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/US-2D.png");
    auto cropper = ImageCropper::create(-1, 0.25)->connect(importer);
    cropper->run();
    auto image = cropper->getOutputData<Image>();
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 128);
}

TEST_CASE("ImageCropper 3D", "[fast][ImageCropper]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");
    auto cropper = ImageCropper::create(Vector3i(32, 34, 36), Vector3i(2, 4, 0))->connect(importer);
    cropper->run();
    auto image = cropper->getOutputData<Image>();
    CHECK(image->getWidth() == 32);
    CHECK(image->getHeight() == 34);
    CHECK(image->getDepth() == 36);
}

TEST_CASE("ImageCropper 3D - No Offset", "[fast][ImageCropper]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");
    auto cropper = ImageCropper::create(Vector3i(32, 34, 36))->connect(importer);
    cropper->run();
    auto image = cropper->getOutputData<Image>();
    CHECK(image->getWidth() == 32);
    CHECK(image->getHeight() == 34);
    CHECK(image->getDepth() == 36);
}


TEST_CASE("ImageCropper 3D - Crop Bottom", "[fast][ImageCropper]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");
    auto cropper = ImageCropper::create(0.25)->connect(importer);
    cropper->run();
    auto image = cropper->getOutputData<Image>();
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 207);
}


TEST_CASE("ImageCropper 3D - Crop Top", "[fast][ImageCropper]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");
    auto cropper = ImageCropper::create(-1, 0.25)->connect(importer);
    cropper->run();
    auto image = cropper->getOutputData<Image>();
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 207);
}