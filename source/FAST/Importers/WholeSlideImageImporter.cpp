#include "WholeSlideImageImporter.hpp"
#include <memory>
#include "FAST/Data/DataTypes.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Data/Image.hpp"
#include <cctype>
#if defined(__APPLE__) || defined(__MACOSX)
#include <openslide.h>
#else
#include <openslide/openslide.h>
#endif
#include <FAST/Data/ImagePyramid.hpp>
#include "TIFFImagePyramidImporter.hpp"


namespace fast {

void WholeSlideImageImporter::readWithTIFF(std::string filename) {
    auto image = TIFFImagePyramidImporter::create(filename)->runAndGetOutputData<ImagePyramid>();
    addOutputData(0, image);
}

void WholeSlideImageImporter::readWithOpenSlide(std::string filename) {
    reportInfo() << "Using OpenSlide version: " << openslide_get_version() << reportEnd();
    openslide_t* file = openslide_open(filename.c_str());
    if(file == nullptr) {
        throw Exception("OpenSlide was unable to open the file: " + filename);
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
    if(levels <= 0)
        throw Exception("Something went wrong when trying to open the WSI with OpenSlide");

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
        try {
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
        } catch(...) {

        }
    }
    reportInfo() << "Spacing set to " << image->getSpacing().transpose() << " millimeters" << reportEnd();
    // Try to get magnification
    try {
        float magnification = std::stof(image->getMetadata("openslide.objective-power"));
        reportInfo() << "Got magnification from OpenSlide: " << magnification << reportEnd();
        image->setMagnification(magnification);
    } catch(...) {
        reportInfo() << "Unable to get magnification using OpenSlide" << reportEnd();
    }

    addOutputData(0, image);
}

void WholeSlideImageImporter::execute() {
    if(m_filename.empty())
        throw Exception("No filename was supplied to the WholeSlideImageImporter");

    if(!fileExists(m_filename))
        throw FileNotFoundException(m_filename);

    std::string lf = stringToLower(m_filename);
    if(
            lf.find(".ome.tiff") != std::string::npos ||
            lf.find(".ome.tif") != std::string::npos ||
            lf.find(".ome.btf") != std::string::npos ||
            lf.find(".tiff") != std::string::npos ||
            lf.find(".tif") != std::string::npos
            ) {
        // OpenSlide doesn't support reading OME-TIFF, use FAST and lbitiff instead.
        // Also FAST supports more TIFF compression formats, thus use FAST and libtiff for regular TIFF files as well.
        try {
            readWithTIFF(m_filename);
        } catch(Exception &e) {
            // Failed to open with FAST and TIFF, try with OpenSlide instead
            reportInfo() << "Failed to open TIFF with FAST, trying OpenSlide instead. Error message: " << e.what() << reportEnd();
            readWithOpenSlide(m_filename);
        }
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
