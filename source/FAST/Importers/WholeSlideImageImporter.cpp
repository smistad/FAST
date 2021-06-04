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

namespace fast {

void WholeSlideImageImporter::readWithOpenSlide(std::string filename) {
    openslide_t* file = openslide_open(filename.c_str());
    if(file == nullptr) {
        const char * message = openslide_get_error(file);
        throw Exception("Unable to open file " + filename + ". OpenSlide error message: " + message);
    }


    // Read metainformation
    auto names = openslide_get_property_names(file);
    int i = 0;
    std::unordered_map<std::string, std::string> metadata;
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
        float spacingX = std::stof(image->getMetadata("openslide.mpp-x")) / 1000.0f;
        float spacingY = std::stof(image->getMetadata("openslide.mpp-y")) / 1000.0f;
        image->setSpacing(Vector3f(spacingX, spacingY, 1.0f));
    } catch(Exception &e) {
        // Try TIFF resolution instead
        float resX = std::stof(image->getMetadata("tiff.XResolution")); // Nr of pixels per unit
        float resY = std::stof(image->getMetadata("tiff.YResolution"));
        std::string unit = image->getMetadata("tiff.ResolutionUnit");
        // Convert to millimeters
        if(unit == "centimeter") {
            resX *= 10.0f;
            resY *= 10.0f;
        } else if(unit == "inch") {
            resX *= 25.5f;
            resY *= 25.5f;
        } else {
            // No unit specified, use as is..
        }
        image->setSpacing(Vector3f(1.0f / resX, 1.0f / resY, 1.0f));
    }

    addOutputData(0, image);
}

void WholeSlideImageImporter::execute() {
    if(m_filename.empty())
        throw Exception("No filename was supplied to the WholeSlideImageImporter");

    if(!fileExists(m_filename))
        throw FileNotFoundException(m_filename);

    //std::string extension = stringToLower(m_filename.substr(m_filename.rfind('.')));
    readWithOpenSlide(m_filename);
}

WholeSlideImageImporter::WholeSlideImageImporter() {
    createOutputPort<ImagePyramid>(0);

    createStringAttribute("filename", "Filename", "Filename to read", "");
    //createBooleanAttribute("grayscale", "Grayscale", "Turn image into grayscale on import", mGrayscale);
}

void WholeSlideImageImporter::loadAttributes() {
    FileImporter::loadAttributes();
}

}
