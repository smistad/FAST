#include "TIFFImagePyramidImporter.hpp"
#include <tiffio.h>
#include <FAST/Data/ImagePyramid.hpp>
#include <regex>

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
        std::string strstr = str.get();
        if(strstr.find("tag ignored") == std::string::npos) // ignore warnings about incorrect tag
            Reporter::warning() << "TIFF: " << module << ": " << str.get() << Reporter::end();
    });
    TIFF* tiff = TIFFOpen(m_filename.c_str(), "rm");
    if(tiff == nullptr) {
        throw Exception("Failed to open file " + m_filename);
    }

    std::vector<ImagePyramidLevel> levelList;
    uint16_t channels;

    bool isOMETiff = false;
    TIFFSetDirectory(tiff, 0);
    // Read description to check if image is OME-TIFF
    char * descriptionChar;
    int result = TIFFGetField(tiff, TIFFTAG_IMAGEDESCRIPTION, &descriptionChar);
    std::string description;
    if(result == 1) { // Must check if tag exists
        description = descriptionChar;
        if(description.find("xml") != std::string::npos && description.find("OME") != std::string::npos) {
            reportInfo() << "TIFF file seems to be an OME-TIFF, reading it as such.." << reportEnd();
            isOMETiff = true;
        }
    }

    float magnification = 0.0f;
    if(isOMETiff) {
        TIFFSetDirectory(tiff, 0);

        // read data for level 0
        uint width, height;
        TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
        uint tileWidth, tileHeight;
        TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tileWidth);
        TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tileHeight);
        TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &channels);
        reportInfo() << "Level " << 0 <<  " has size " << width << " " << height <<
        " and tile size: " << tileWidth << " " << tileHeight << " channels " << channels << reportEnd();

        ImagePyramidLevel levelData;
        levelData.width = width;
        levelData.height = height;
        levelData.tileWidth = tileWidth;
        levelData.tileHeight = tileHeight;
        levelData.offset = 0;
        levelList.push_back(levelData);

        // Read SubIFD offsets
        uint16_t count;
        uint64_t* values; // If big tiff, this is 64. This is deleted by TIFF
        TIFFGetField(tiff, TIFFTAG_SUBIFD, &count, &values);
        std::vector<uint64_t> offsets; // Have to copy the offsets for some reason
        for(int i = 0; i < count; ++i)
            offsets.push_back(values[i]);

        reportInfo() << "Found " << count << " subdirectories (SubIFD) in OME-TIFF file" << reportEnd();
        for(int i = 0; i < count; ++i) {
            TIFFSetSubDirectory(tiff, offsets[i]);
            uint width, height;
            TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
            TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
            uint tileWidth, tileHeight;
            TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tileWidth);
            TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tileHeight);
            reportInfo() << "Level " << (i+1) <<  " has size " << width << " " << height <<
                         " and tile size: " << tileWidth << " " << tileHeight << " channels " << channels << reportEnd();

            ImagePyramidLevel levelData;
            levelData.width = width;
            levelData.height = height;
            levelData.tileWidth = tileWidth;
            levelData.tileHeight = tileHeight;
            levelData.offset = offsets[i];
            levelList.push_back(levelData);
        }

        // Try to get magnification from description using regex
        std::regex pattern(R"(NominalMagnification="([-+]?[0-9]*\.?[0-9]+))");
        std::smatch match;
        std::string str = description;
        magnification = 0.0f;
        while(std::regex_search(str, match, pattern)) {
            if(match.size() >= 2) {
                if(std::stof(match[1]) > magnification) { // Get largest magnification
                    magnification = std::stof(match[1]);
                }
            }
            str = match.suffix();
        }
        if(magnification == 0) {
            reportInfo() << "Unable to get magnification from OME-TIFF XML data" << reportEnd();
        } else {
            reportInfo() << "Got magnification " << magnification << " from OME-TIFF XML data" << reportEnd();
        }
    } else {
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
    }
    auto image = ImagePyramid::create(tiff, levelList, (int)channels, isOMETiff);
    if(magnification > 0)
        image->setMagnification(magnification);
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