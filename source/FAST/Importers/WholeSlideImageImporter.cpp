#include "WholeSlideImageImporter.hpp"
#include <memory>
#include "FAST/Data/DataTypes.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Data/Image.hpp"
#include <cctype>
#include <algorithm>
#include <openslide.h>

namespace fast {

void WholeSlideImageImporter::execute() {
    if (mFilename == "")
        throw Exception("No filename was supplied to the WholeSlideImageImporter");

    if(not fileExists(mFilename))
        throw FileNotFoundException();

    openslide_t* file = openslide_open(mFilename.c_str());
    if(file == nullptr) {
        const char * message = openslide_get_error(file);
        throw Exception("Unable to open file " + mFilename + ". OpenSlide error message: " + message);
    }

    // Get total size of image
    int64_t fullWidth = -1, fullHeight = -1;
    openslide_get_level_dimensions(file, 2, &fullWidth, &fullHeight); // Level 0 is the largest level
    reportInfo() << "WSI image size: " << fullWidth << ", " << fullHeight << reportEnd();

    const char * pixelSpacingX = openslide_get_property_value(file, OPENSLIDE_PROPERTY_NAME_MPP_X);
    const char * pixelSpacingY = openslide_get_property_value(file, OPENSLIDE_PROPERTY_NAME_MPP_Y);
    if(pixelSpacingX != nullptr and pixelSpacingY != nullptr)
        reportInfo() << "WSI pixel spacing: " << pixelSpacingX << ", " << pixelSpacingY << " microns per pixel" << reportEnd();

    //const char * boundsWidth = openslide_get_property_value(file, OPENSLIDE_PROPERTY_NAME_BOUNDS_WIDTH);
    //const char * boundsHeight = openslide_get_property_value(file, OPENSLIDE_PROPERTY_NAME_BOUNDS_HEIGHT);

    int size = 16000;
    auto data = make_uninitialized_unique<uint8_t[]>(fullWidth*fullHeight*4);
    openslide_read_region(file, (uint32_t*)data.get(), 0, 0, 2, fullWidth, fullHeight);

    // Create image
    auto image = getOutputData<Image>();
    image->create(fullWidth, fullHeight, TYPE_UINT8, 4, Host::getInstance(), std::move(data));

    openslide_close(file);
}

WholeSlideImageImporter::WholeSlideImageImporter() {
    mFilename = "";
    mIsModified = true;
    mGrayscale = false;
    createOutputPort<Image>(0);
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
