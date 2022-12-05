#include "WholeSlideImageImporter.hpp"
#include <memory>
#include "FAST/Data/DataTypes.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Data/Image.hpp"
#include <cctype>
#include <algorithm>
#include <openslide/openslide.h>
#include <FAST/Data/ImagePyramid.hpp>
#include <fstream>
#include <jpeglib.h>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>


namespace fast {

struct SIS_header {
    char magic[4]; // SIS0
    uint32_t headerSize;
    uint32_t version;
    uint32_t Ndim;
    uint64_t etsoffset;
    uint32_t etsnbytes;
    uint32_t dummy0; // reserved
    uint64_t offsettiles;
    uint32_t ntiles;
    uint32_t dummy1; // reserved
    uint32_t dummy2; // reserved
    uint32_t dummy3; // reserved
    uint32_t dummy4; // reserved
    uint32_t dummy5; // reserved
};

struct ETS_header {
    char magic[4]; // ETS0
    uint32_t version;
    uint32_t pixelType;
    uint32_t sizeC;
    uint32_t colorspace;
    uint32_t compression;
    uint32_t quality;
    uint32_t dimx;
    uint32_t dimy;
    uint32_t dimz;
};

#define READ(X) \
    stream->read((char*)&X, sizeof(X)); \
    //std::cout << # X ": " << X << std::endl; \

void WholeSlideImageImporter::readVSI(std::string filename) {
    std::string originalFilename = getFileName(filename);
    std::string directoryName = getDirName(filename) + "_" + originalFilename.substr(0, originalFilename.size()-4) + "_/";
    std::string etsFilename = "";
    for(auto folder : getDirectoryList(directoryName, false, true)) {
        if(fileExists(join(directoryName, folder, "frame_t.ets"))) {
            if(!etsFilename.empty()) {
                // If multiple files: use the one which have correct compression.. (normal lossy JPEG)
                auto stream = new std::ifstream(join(directoryName, folder, "frame_t.ets"), std::ifstream::binary | std::ifstream::in);
                if(!stream->is_open())
                    continue;
                SIS_header sis_header;

                READ(sis_header.magic)
                READ(sis_header.headerSize)
                READ(sis_header.version)
                READ(sis_header.Ndim)
                READ(sis_header.etsoffset)
                READ(sis_header.etsnbytes)
                READ(sis_header.dummy0)
                READ(sis_header.offsettiles)
                READ(sis_header.ntiles)
                READ(sis_header.dummy1)
                READ(sis_header.dummy2)
                READ(sis_header.dummy3)
                READ(sis_header.dummy4)
                READ(sis_header.dummy5)

                ETS_header ets_header;
                READ(ets_header.magic)
                READ(ets_header.version)
                READ(ets_header.pixelType)
                READ(ets_header.sizeC)
                READ(ets_header.colorspace)
                READ(ets_header.compression)
                READ(ets_header.quality)
                READ(ets_header.dimx)
                READ(ets_header.dimy)
                READ(ets_header.dimz)

                if(ets_header.compression == 2 || ets_header.compression == 0)
                    etsFilename = join(directoryName, folder, "frame_t.ets");
                stream->close();
                delete stream;
            } else {
                etsFilename = join(directoryName, folder, "frame_t.ets");
            }
        }
    }
    if(etsFilename.empty())
        throw Exception("Could not find frame_t.ets file under " + directoryName + " while importing " + filename);

    std::ifstream* stream = new std::ifstream(etsFilename.c_str(), std::ifstream::binary | std::ifstream::in);
    if(!stream->is_open())
        throw Exception("Unable to open file!");
    SIS_header sis_header;

    READ(sis_header.magic)
    READ(sis_header.headerSize)
    READ(sis_header.version)
    READ(sis_header.Ndim)
    READ(sis_header.etsoffset)
    READ(sis_header.etsnbytes)
    READ(sis_header.dummy0)
    READ(sis_header.offsettiles)
    READ(sis_header.ntiles)
    READ(sis_header.dummy1)
    READ(sis_header.dummy2)
    READ(sis_header.dummy3)
    READ(sis_header.dummy4)
    READ(sis_header.dummy5)

    ETS_header ets_header;
    READ(ets_header.magic)
    READ(ets_header.version)
    READ(ets_header.pixelType)
    READ(ets_header.sizeC)
    READ(ets_header.colorspace)
    READ(ets_header.compression)
    READ(ets_header.quality)
    READ(ets_header.dimx)
    READ(ets_header.dimy)
    READ(ets_header.dimz)

    if(ets_header.compression != 2 && ets_header.compression != 0)
        throw Exception("Importing Olympus VSI with another compression format than RAW or JPEG is not supported yet. Compress format: " + std::to_string(ets_header.compression));
    ImageCompression compressionFormat;
    if(ets_header.compression == 2) {
        compressionFormat = ImageCompression::JPEG;
        reportInfo() << "VSI was compressed with JPEG" << reportEnd();
    } else if(ets_header.compression == 0) {
        compressionFormat = ImageCompression::RAW;
        reportInfo() << "VSI was not compressed (RAW)" << reportEnd();
    }

    stream->seekg(sis_header.offsettiles);
    std::vector<vsi_tile_header> tiles;
    int maxLevel = 0;
    std::map<int, std::pair<int, int>> maxTiles;
    for(int i = 0; i < sis_header.ntiles; ++i) {
        vsi_tile_header tile_header;
        READ(tile_header.dummy1)
        READ(tile_header.coord)
        READ(tile_header.level)
        READ(tile_header.offset)
        READ(tile_header.numbytes)
        READ(tile_header.dummy2)
        tiles.push_back(tile_header);
        if(maxTiles.count(tile_header.level) == 0) {
            maxTiles[tile_header.level].first = tile_header.coord[0];
            maxTiles[tile_header.level].second = tile_header.coord[1];
        } else {
            maxTiles[tile_header.level].first = std::max(maxTiles[tile_header.level].first, (int)tile_header.coord[0]);
            maxTiles[tile_header.level].second = std::max(maxTiles[tile_header.level].second, (int)tile_header.coord[1]);
        }
        maxLevel = std::max(maxLevel, (int)tile_header.level);
    }

    // This format does necessarily have the same aspect ratio for each level. And downsampling factor varies from level to level
    // Thus we create a fake pyramid which is half downsampled for each level from level 0. Any tiles or data requested outside should be blank.
    // The format does not seem to store tiles which are just glass at a high-resolution.
    std::vector<ImagePyramidLevel> levelList;
    int fullWidth = (maxTiles[0].first+1)*ets_header.dimx;
    int fullHeight = (maxTiles[0].second+1)*ets_header.dimy;
    for(int level = 0; level <= maxLevel; ++level) {
        ImagePyramidLevel levelData;
        levelData.tileWidth = ets_header.dimx;
        levelData.tileHeight = ets_header.dimy;
        //levelData.width = (maxTiles[level].first+1)*ets_header.dimx;
        //levelData.height = (maxTiles[level].second+1)*ets_header.dimy;
        levelData.width = fullWidth;
        levelData.height = fullHeight;
        reportInfo() << "VSI level: " << level << ": " << levelData.width << " " << levelData.height << reportEnd();
        levelData.tilesX = std::ceil((float)fullWidth / ets_header.dimx);
        levelData.tilesY = std::ceil((float)fullHeight / ets_header.dimy);
        fullWidth /= 2;
        fullHeight /= 2;
        if(levelData.width < 1024 || levelData.height < 1024) // Skip very small levels
            break;
        levelList.push_back(levelData);
    }

    auto image = ImagePyramid::create(stream, tiles, levelList, compressionFormat);

    addOutputData(0, image);
}

void WholeSlideImageImporter::readWithOpenSlide(std::string filename) {
    openslide_t* file = openslide_open(filename.c_str());
    if(file == nullptr) {
        const char * message = openslide_get_error(file);
        throw Exception("Unable to open file " + filename + ". OpenSlide error message: " + message);
    }

    // Read metainformation
    auto names = openslide_get_property_names(file);
    int i = 0;
    std::map<std::string, std::string> metadata;
    while(names[i] != nullptr) {
        std::string name = names[i];
        std::string value = openslide_get_property_value(file, names[i]);
        i++;
        reportInfo() << "Metadata: " << name << " = " << value << reportEnd();
        metadata[name] = value;
    }

    std::vector<ImagePyramidLevel> levelList;
    int levels = openslide_get_level_count(file);
    reportInfo() << "WSI has " << levels << " levels" << reportEnd();

    // Find out how many levels to skip
    for(int level = 0; level < levels; ++level) {
        reportInfo() << "Checking level " << level << reportEnd();
        // Get total size of image
        int64_t fullWidth = -1, fullHeight = -1;
        openslide_get_level_dimensions(file, level, &fullWidth, &fullHeight); // Level 0 is the largest level
        std::size_t bytes = fullWidth * fullHeight * 4;
        int sizeInMB = bytes / (1024 * 1024);
        if(sizeInMB > 4) {
            reportInfo() << "WSI level " << level << " has size " << fullWidth << "x" << fullHeight << " and " << sizeInMB << " MB adding.." << reportEnd();
            ImagePyramidLevel levelData;
            levelData.width = fullWidth;
            levelData.height = fullHeight;
            try {
                levelData.tileWidth = std::stoi(metadata.at("openslide.level[" + std::to_string(level) + "].tile-width"));
                levelData.tileHeight = std::stoi(metadata.at("openslide.level[" + std::to_string(level) + "].tile-height"));
                reportInfo() << "Setting tile width and height to " << levelData.tileWidth << " " << levelData.tileHeight << reportEnd();
            } catch(...) {
            }
            levelList.push_back(levelData);
        } else {
            reportInfo() << "WSI level was less than 4 MB, skipping.." << reportEnd();
            break;
        }
    }
    auto image = ImagePyramid::create(file, levelList);
    image->setMetadata(metadata);

    try {
        // Try to get spacing in microns from openslide, and convert to millimeters.
        reportInfo() << "Trying to get spacing as openslide.mpp: " << reportEnd();
        float spacingX = std::stof(image->getMetadata("openslide.mpp-x")) / 1000.0f;
        float spacingY = std::stof(image->getMetadata("openslide.mpp-y")) / 1000.0f;
        image->setSpacing(Vector3f(spacingX, spacingY, 1.0f));
    } catch(Exception &e) {
        // Try TIFF resolution instead
        // Definition of TIFF resolution is: The number of pixels per ResolutionUnit (https://www.awaresystems.be/imaging/tiff/tifftags/xresolution.html)
        reportInfo() << "Trying to get spacing from tiff.X/YResolution: " << reportEnd();
        float resX = std::stof(image->getMetadata("tiff.XResolution")); // Nr of pixels per unit
        float resY = std::stof(image->getMetadata("tiff.YResolution"));
        std::string unit = image->getMetadata("tiff.ResolutionUnit");
        // Convert to # pixels per millimeters
        if(unit == "centimeter") {
            resX /= 10.0f;
            resY /= 10.0f;
        } else if(unit == "inch") {
            resX /= 25.5f;
            resY /= 25.5f;
        } else {
            // No unit specified, use as is..
        }
        // Convert #pixels per millimeters to spacing of each pixel
        image->setSpacing(Vector3f(1.0f / resX, 1.0f / resY, 1.0f));
    }
    reportInfo() << "Spacing set to " << image->getSpacing().transpose() << " millimeters" << reportEnd();

    addOutputData(0, image);
}

void WholeSlideImageImporter::execute() {
    if(m_filename.empty())
        throw Exception("No filename was supplied to the WholeSlideImageImporter");

    if(!fileExists(m_filename))
        throw FileNotFoundException(m_filename);

    std::string extension = stringToLower(m_filename.substr(m_filename.rfind('.')));
    if(extension == ".vsi") {
        readVSI(m_filename);
    } else {
        readWithOpenSlide(m_filename);
    }
}

WholeSlideImageImporter::WholeSlideImageImporter() {
    createOutputPort<ImagePyramid>(0);
}
WholeSlideImageImporter::WholeSlideImageImporter(std::string filename) : FileImporter(filename) {
    createOutputPort<ImagePyramid>(0);
}
void WholeSlideImageImporter::loadAttributes() {
    FileImporter::loadAttributes();
}

}
