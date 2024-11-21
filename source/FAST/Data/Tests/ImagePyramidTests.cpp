#include <FAST/Testing.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>

using namespace fast;

TEST_CASE("Convert image pyramid to JPEG XL compression", "[fast][ImagePyramid][JPEGXL]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/CMU-1.svs");
    const int level = 2;
    const int targetTileSize = 512;
    auto imagePyramid = importer->runAndGetOutputData<ImagePyramid>();
    int tilesX = std::ceil(imagePyramid->getLevelWidth(level) / targetTileSize);
    int tilesY = std::ceil(imagePyramid->getLevelHeight(level) / targetTileSize);

    auto newImagePyramid = ImagePyramid::create(tilesX*targetTileSize, tilesY*targetTileSize, 3, targetTileSize, targetTileSize, ImageCompression::JPEGXL);

    auto accessRead = imagePyramid->getAccess(ACCESS_READ);
    auto accessWrite = newImagePyramid->getAccess(ACCESS_READ_WRITE);

    int counter = 0;
    for(int newLevel = 0; newLevel < newImagePyramid->getNrOfLevels(); ++newLevel) {
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

                    accessWrite->setPatch(newLevel, x, y, patch, false);
                } catch(Exception &e) {
                    continue;
                }
                ++counter;
                std::cout << "Progress: " << counter << " of " << totalTiles << std::endl;

            }
        }
    }
}