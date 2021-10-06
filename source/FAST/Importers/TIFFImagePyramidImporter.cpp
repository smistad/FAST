#include "TIFFImagePyramidImporter.hpp"
#include <tiffio.h>
#include <FAST/Data/ImagePyramid.hpp>

namespace fast {

void TIFFImagePyramidImporter::execute() {
    reportInfo() << "Reading WSI with TIFF" << reportEnd();
    TIFFSetErrorHandler([](const char* module, const char* fmt, va_list ap) {
        auto str = make_uninitialized_unique<char[]>(512);
        sprintf(str.get(), fmt, ap);
        Reporter::warning() << "TIFF: " << module << ": " << str.get() << Reporter::end();
    });
    TIFFSetWarningHandler([](const char* module, const char* fmt, va_list ap) {
        auto str = make_uninitialized_unique<char[]>(512);
        sprintf(str.get(), fmt, ap);
        Reporter::warning() << "TIFF: " << module << ": " << str.get() << Reporter::end();
    });
    TIFF* tiff = TIFFOpen(m_filename.c_str(), "rm");
    if(tiff == nullptr) {
        throw Exception("Failed to open file " + m_filename);
    }

    std::vector<ImagePyramidLevel> levelList;
    uint16_t channels;
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
             " and tile size: " << tileWidth << " " << tileHeight << " channels " << channels << reportEnd();

        ImagePyramidLevel levelData;
        levelData.width = width;
        levelData.height = height;
        levelData.tileWidth = tileWidth;
        levelData.tileHeight = tileHeight;
        levelList.push_back(levelData);
    }
    auto image = ImagePyramid::create(tiff, levelList, (int)channels);
    addOutputData(0, image);
}

TIFFImagePyramidImporter::TIFFImagePyramidImporter() {
    createOutputPort<ImagePyramid>(0);
}
TIFFImagePyramidImporter::TIFFImagePyramidImporter(std::string filename) : FileImporter(filename) {
    createOutputPort<ImagePyramid>(0);
}

void TIFFImagePyramidImporter::loadAttributes() {
    FileImporter::loadAttributes();
}

}