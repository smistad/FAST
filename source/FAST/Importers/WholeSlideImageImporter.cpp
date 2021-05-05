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

    auto image = ImagePyramid::New();

    // Read metainformation
    auto names = openslide_get_property_names(file);
    int i = 0;
    while(names[i] != nullptr) {
        std::string name = names[i];
        std::string value = openslide_get_property_value(file, names[i]);
        i++;
        reportInfo() << "Metadata: " << name << " = " << value << reportEnd();
        image->setMetadata(name, value);
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
                levelData.tileWidth = std::stoi(image->getMetadata("openslide.level[" + std::to_string(level) + "].tile-width"));
                levelData.tileHeight = std::stoi(image->getMetadata("openslide.level[" + std::to_string(level) + "].tile-height"));
                reportInfo() << "Setting tile width and height to " << levelData.tileWidth << " " << levelData.tileHeight << reportEnd();
            } catch(...) {
            }
            levelList.push_back(levelData);
        } else {
            reportInfo() << "WSI level was less than 4 MB, skipping.." << reportEnd();
            break;
        }
    }
    image->create(file, levelList);
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
