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

void WholeSlideImageImporter::execute() {
    if (mFilename.empty())
        throw Exception("No filename was supplied to the WholeSlideImageImporter");

    if(!fileExists(mFilename))
        throw FileNotFoundException(mFilename);

    openslide_t* file = openslide_open(mFilename.c_str());
    if(file == nullptr) {
        const char * message = openslide_get_error(file);
        throw Exception("Unable to open file " + mFilename + ". OpenSlide error message: " + message);
    }

    auto image = getOutputData<ImagePyramid>();

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
            levelList.push_back(levelData);
        } else {
            reportInfo() << "WSI level was less than 4 MB, skipping.." << reportEnd();
            break;
        }
    }
    image->create(file, levelList);
}

WholeSlideImageImporter::WholeSlideImageImporter() {
    mFilename = "";
    mIsModified = true;
    mGrayscale = false;
    createOutputPort<ImagePyramid>(0);

    createStringAttribute("filename", "Filename", "Filename to read", "");
    //createBooleanAttribute("grayscale", "Grayscale", "Turn image into grayscale on import", mGrayscale);
}

void WholeSlideImageImporter::loadAttributes() {
    setFilename(getStringAttribute("filename"));
}

void WholeSlideImageImporter::setGrayscale(bool grayscale) {
    mGrayscale = grayscale;
    mIsModified = true;
}

void WholeSlideImageImporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

}
