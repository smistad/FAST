#include "TIFFImagePyramidImporter.hpp"
#include <tiffio.h>
#include <FAST/Data/ImagePyramid.hpp>

namespace fast {

void TIFFImagePyramidImporter::execute() {
    reportInfo() << "Reading WSI with TIFF" << reportEnd();
    TIFF* tiff = TIFFOpen(m_filename.c_str(), "rm");
    if(tiff == nullptr) {
        throw Exception("Failed to open file " + m_filename);
    }

    std::vector<ImagePyramidLevel> levelList;
    uint channels;
    for(int level = 0; level < TIFFNumberOfDirectories(tiff); ++level) {
        TIFFSetDirectory(tiff, level);
        uint width, height;
        TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
        uint tileWidth, tileHeight;
        TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tileWidth);
        TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tileHeight);
        TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &channels);
        reportInfo() << "Level " << level <<  " has size " << width << " " << height <<
             " and tile size: " << tileWidth << " " << tileHeight << reportEnd();

        ImagePyramidLevel levelData;
        levelData.width = width;
        levelData.height = height;
        levelData.tileWidth = tileWidth;
        levelData.tileHeight = tileHeight;
        levelList.push_back(levelData);
    }
    auto image = ImagePyramid::New();
    image->create(tiff, levelList, channels);
    addOutputData(0, image);
}

TIFFImagePyramidImporter::TIFFImagePyramidImporter() {
    createOutputPort<ImagePyramid>(0);
}

void TIFFImagePyramidImporter::loadAttributes() {
    FileImporter::loadAttributes();
}

}