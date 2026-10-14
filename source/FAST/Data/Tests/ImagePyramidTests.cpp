#include <FAST/Testing.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Exporters/TIFFImagePyramidExporter.hpp>
#include <FAST/Importers/TIFFImagePyramidImporter.hpp>

using namespace fast;

TEST_CASE("Convert image pyramid to JPEG XL compression", "[fast][ImagePyramid][JPEG]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/CMU-1.svs");
    const int level = 2;
    const int targetTileSize = 512;
    auto imagePyramid = importer->runAndGetOutputData<ImagePyramid>();
    int tilesX = std::ceil(imagePyramid->getLevelWidth(level) / targetTileSize);
    int tilesY = std::ceil(imagePyramid->getLevelHeight(level) / targetTileSize);

    auto newImagePyramid = ImagePyramid::create(
            tilesX*targetTileSize,
            tilesY*targetTileSize,
            3,
            targetTileSize,
            targetTileSize,
            ImageCompression::JPEGXL
    );

    {
        auto accessRead = imagePyramid->getAccess(ACCESS_READ);
        auto accessWrite = newImagePyramid->getAccess(ACCESS_READ_WRITE);

        int counter = 0;
        for(int newLevel = 0; newLevel < newImagePyramid->getNrOfLevels(); ++newLevel) {
            //int newLevel = 0;
            tilesX = std::ceil(newImagePyramid->getLevelWidth(newLevel) / targetTileSize);
            tilesY = std::ceil(newImagePyramid->getLevelHeight(newLevel) / targetTileSize);
            int totalTiles = tilesX*tilesY;
            for(int tileX = 0; tileX < tilesX; ++tileX) {
                for(int tileY = 0; tileY < tilesY; ++tileY) {
                    try {
                        auto x = tileX*targetTileSize;
                        auto y = tileY*targetTileSize;
                        int pow = (int)std::pow(2, newLevel);
                        auto patch = accessRead->getPatchAsImage(level, x*pow, y*pow, targetTileSize*pow, targetTileSize*pow);
                        if(newLevel != 0)
                            patch = ImageResizer::create(targetTileSize, targetTileSize)->connect(patch)->runAndGetOutputData<Image>();

                        if(tileX % 2 == 1) {
                            accessWrite->setBlankPatch(newLevel, x, y);
                        } else {
                            accessWrite->setPatch(newLevel, x, y, patch, true);
                        }
                    } catch(Exception &e) {
                        continue;
                    }
                    ++counter;
                    std::cout << "Progress: " << counter << " of " << totalTiles << std::endl;
                }
            }
        }
    }
    std::cout << "Done" << std::endl;

    TIFFImagePyramidExporter::create("test.tiff")->connect(newImagePyramid)->run();

    //auto importer2 = TIFFImagePyramidImporter::create("test.tiff");
    ////auto renderer = ImagePyramidRenderer::create()->connect(newImagePyramid);
    //auto renderer = ImagePyramidRenderer::create()->connect(importer2);
    //SimpleWindow2D::create()->connect(renderer)->run();
}

TEST_CASE("getMagnification()", "[fast][ImagePyramid]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/CMU-1.svs");
    auto image = importer->runAndGetOutputData<ImagePyramid>();
    CHECK(image->getMagnification() == 20);
}

TEST_CASE("setMagnification()", "[fast][ImagePyramid]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/CMU-1.svs");
    auto image = importer->runAndGetOutputData<ImagePyramid>();
    image->setMagnification(40);
    CHECK(image->getMagnification() == 40);
}

TEST_CASE("getLevelForMagnification()", "[fast][ImagePyramid]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/CMU-1.svs");
    auto image = importer->runAndGetOutputData<ImagePyramid>();
    CHECK(image->getLevelForMagnification(20) == 0);
    CHECK(image->getLevelForMagnification(5) == 1);
    CHECK(image->getLevelForMagnification(1.25) == 2);

    CHECK_THROWS(image->getLevelForMagnification(10));
}

TEST_CASE("getClosestLevelForMagnification()", "[fast][ImagePyramid]") {
    // This pyramid is 20X, and have levels for 5X and 1.25X
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/CMU-1.svs");
    auto image = importer->run()->getOutput<ImagePyramid>();
    {
        auto [level, resampleFactor] = image->getClosestLevelForMagnification(20);
        CHECK(level == 0);
        CHECK(resampleFactor == 1.0f);
    }
    {
        auto [level, resampleFactor] = image->getClosestLevelForMagnification(15);
        CHECK(level == 0);
        CHECK(resampleFactor == Approx(1.33333f));
    }
    {
        auto [level, resampleFactor] = image->getClosestLevelForMagnification(10);
        CHECK(level == 0);
        CHECK(resampleFactor == 2.0f);
    }
    {
        auto [level, resampleFactor] = image->getClosestLevelForMagnification(5);
        CHECK(level == 1);
        CHECK(resampleFactor == 1.0f);
    }
    {
        auto [level, resampleFactor] = image->getClosestLevelForMagnification(2.5);
        CHECK(level == 1);
        CHECK(resampleFactor == 2.0f);
    }
    {
        auto [level, resampleFactor] = image->getClosestLevelForMagnification(1.25);
        CHECK(level == 2);
        CHECK(resampleFactor == 1.0f);
    }
    {
        auto [level, resampleFactor] = image->getClosestLevelForMagnification(1);
        CHECK(level == 2);
        CHECK(resampleFactor == 1.25f);
    }
    {
        auto [level, resampleFactor] = image->getClosestLevelForMagnification(0.625);
        CHECK(level == 2);
        CHECK(resampleFactor == 2.0f);
    }
}

TEST_CASE("Create image pyramid with default levels", "[fast][ImagePyramid]") {
    auto pyramid = ImagePyramid::create(12000, 8000, 3, 512, 512, ImageCompression::JPEG);

    CHECK(pyramid->getNrOfLevels() == 3);
    CHECK(pyramid->getWidth() == 12000);
    CHECK(pyramid->getHeight() == 8000);
    CHECK(pyramid->getLevelWidth(1) == fast::round(12000.0f/2.0f));
    CHECK(pyramid->getLevelHeight(1) == fast::round(8000.0f/2.0f));
    CHECK(pyramid->getLevelWidth(2) == fast::round(12000.0f/4.0f));
    CHECK(pyramid->getLevelHeight(2) == fast::round(8000.0f/4.0f));
    CHECK_THROWS(pyramid->getLevelWidth(3));
    CHECK(pyramid->getDataType() == TYPE_UINT8);
    CHECK(pyramid->getLevelTileWidth(0) == 512);
    CHECK(pyramid->getLevelTileHeight(0) == 512);
    CHECK(pyramid->getCompression() == ImageCompression::JPEG);
    CHECK(pyramid->getNrOfChannels() == 3);
    CHECK(pyramid->getLevelScale(0) == Approx(1));
    CHECK(pyramid->getLevelScale(1) == Approx(2));
    CHECK(pyramid->getLevelScale(2) == Approx(4));
}

TEST_CASE("Create image pyramid with custom levels", "[fast][ImagePyramid]") {
    auto pyramid = ImagePyramid::create(22000, 13000, 1, 512, 256, ImageCompression::JPEG, 90,
                                        fast::TYPE_UINT8, {4, 2, 2});

    CHECK(pyramid->getNrOfLevels() == 4);
    CHECK(pyramid->getWidth() == 22000);
    CHECK(pyramid->getHeight() == 13000);
    CHECK(pyramid->getLevelWidth(1) == fast::round(22000.0f/4.0f));
    CHECK(pyramid->getLevelHeight(1) == fast::round(13000.0f/4.0f));
    CHECK(pyramid->getLevelWidth(2) == fast::round(22000.0f/(4.0f*2.0f)));
    CHECK(pyramid->getLevelHeight(2) == fast::round(13000.0f/(4.0f*2.0f)));
    CHECK(pyramid->getLevelWidth(3) == fast::round(22000.0f/(4.0f*2.0f*2.0f)));
    CHECK(pyramid->getLevelHeight(3) == fast::round(13000.0f/(4.0f*2.0f*2.0f)));
    CHECK_THROWS(pyramid->getLevelWidth(4));
    CHECK(pyramid->getDataType() == TYPE_UINT8);
    CHECK(pyramid->getLevelTileWidth(0) == 512);
    CHECK(pyramid->getLevelTileHeight(0) == 256);
    CHECK(pyramid->getCompression() == ImageCompression::JPEG);
    CHECK(pyramid->getNrOfChannels() == 1);
    CHECK(pyramid->getLevelScale(0) == Approx(1));
    CHECK(pyramid->getLevelScale(1) == Approx(4));
    CHECK(pyramid->getLevelScale(2) == Approx(8));
    CHECK(pyramid->getLevelScale(3) == Approx(16));
}

TEST_CASE("Create image pyramid with custom level sizes", "[fast][ImagePyramid]") {
    std::vector<Vector2i> sizes = {
            {22000/1.5f, 13000/1.5f},
            {22000/4.5f, 13000/4.5f}
    };
    auto pyramid = ImagePyramid::create(22000, 13000, 1, 512, 512, ImageCompression::JPEG, 90,
                                        fast::TYPE_UINT8, {}, sizes);

    CHECK(pyramid->getNrOfLevels() == 3);
    CHECK(pyramid->getWidth() == 22000);
    CHECK(pyramid->getHeight() == 13000);
    CHECK(pyramid->getLevelWidth(1) == sizes[0].x());
    CHECK(pyramid->getLevelHeight(1) == sizes[0].y());
    CHECK(pyramid->getLevelWidth(2) == sizes[1].x());
    CHECK(pyramid->getLevelHeight(2) == sizes[1].y());
    CHECK_THROWS(pyramid->getLevelWidth(3));
    CHECK(pyramid->getDataType() == TYPE_UINT8);
    CHECK(pyramid->getLevelTileWidth(0) == 512);
    CHECK(pyramid->getLevelTileHeight(0) == 512);
    CHECK(pyramid->getCompression() == ImageCompression::JPEG);
    CHECK(pyramid->getNrOfChannels() == 1);
    CHECK(pyramid->getLevelScale(0) == Approx(1.0f).margin(0.01));
    CHECK(pyramid->getLevelScale(1) == Approx(1.5f).margin(0.01));
    CHECK(pyramid->getLevelScale(2) == Approx(4.5f).margin(0.01));
}

TEST_CASE("Create image pyramid with custom level sizes including level 0", "[fast][ImagePyramid]") {
    std::vector<Vector2i> sizes = {
            {22000, 13000},
            {22000/1.5f, 13000/1.5f},
            {22000/4.5f, 13000/4.5f}
    };
    auto pyramid = ImagePyramid::create(22000, 13000, 1, 512, 512, ImageCompression::JPEG, 90,
                                        fast::TYPE_UINT8, {}, sizes);

    CHECK(pyramid->getNrOfLevels() == 3);
    CHECK(pyramid->getWidth() == 22000);
    CHECK(pyramid->getHeight() == 13000);
    CHECK(pyramid->getLevelWidth(1) == sizes[1].x());
    CHECK(pyramid->getLevelHeight(1) == sizes[1].y());
    CHECK(pyramid->getLevelWidth(2) == sizes[2].x());
    CHECK(pyramid->getLevelHeight(2) == sizes[2].y());
    CHECK_THROWS(pyramid->getLevelWidth(3));
    CHECK(pyramid->getDataType() == TYPE_UINT8);
    CHECK(pyramid->getLevelTileWidth(0) == 512);
    CHECK(pyramid->getLevelTileHeight(0) == 512);
    CHECK(pyramid->getCompression() == ImageCompression::JPEG);
    CHECK(pyramid->getNrOfChannels() == 1);
    CHECK(pyramid->getLevelScale(0) == Approx(1.0f).margin(0.01));
    CHECK(pyramid->getLevelScale(1) == Approx(1.5f).margin(0.01));
    CHECK(pyramid->getLevelScale(2) == Approx(4.5f).margin(0.01));
}

TEST_CASE("Invalid level sizes", "[fast][ImagePyramid]") {
    std::vector<Vector2i> sizes = {
            {24000, 13000},
            {128, 128}
    };
    CHECK_THROWS(
        auto pyramid = ImagePyramid::create(22000, 13000, 1, 512, 512, ImageCompression::JPEG, 90,
                                        fast::TYPE_UINT8, {}, sizes);
    );

}

TEST_CASE("getLevelSizes", "[fast][ImagePyramid]") {
    std::vector<Vector2i> sizes = {
            {22000, 13000},
            {22000/1.5f, 13000/1.5f},
            {22000/4.5f, 13000/4.5f}
    };
    auto pyramid = ImagePyramid::create(22000, 13000, 1, 512, 512, ImageCompression::JPEG, 90,
                                        fast::TYPE_UINT8, {}, sizes);
    auto levelSizes = pyramid->getLevelSizes();
    for(int i = 0; i < 3; ++i) {
        CHECK(levelSizes[i] == sizes[i]);
    }
}
