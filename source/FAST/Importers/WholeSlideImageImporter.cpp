#include "WholeSlideImageImporter.hpp"
#include <memory>
#include "FAST/Data/DataTypes.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Data/Image.hpp"
#include <cctype>
#include <algorithm>
#include <openslide.h>
#include <FAST/Data/WholeSlideImage.hpp>

namespace fast {

void WholeSlideImageImporter::execute() {
    if (mFilename == "")
        throw Exception("No filename was supplied to the WholeSlideImageImporter");

    if(not fileExists(mFilename))
        throw FileNotFoundException();

    enableRuntimeMeasurements();
    mRuntimeManager->startRegularTimer("open");
    openslide_t* file = openslide_open(mFilename.c_str());
    mRuntimeManager->stopRegularTimer("open");
    if(file == nullptr) {
        const char * message = openslide_get_error(file);
        throw Exception("Unable to open file " + mFilename + ". OpenSlide error message: " + message);
    }

    std::vector<WholeSlideImageLevel> levelList;
    int levels = openslide_get_level_count(file);
    for(int level = 5; level >= 2; --level) {
        // Get total size of image
        int64_t fullWidth = -1, fullHeight = -1;
        openslide_get_level_dimensions(file, level, &fullWidth, &fullHeight); // Level 0 is the largest level
        reportInfo() << "WSI has " << levels << " levels" << reportEnd();
        reportInfo() << "WSI image size: " << fullWidth << ", " << fullHeight << reportEnd();

        const char *pixelSpacingX = openslide_get_property_value(file, OPENSLIDE_PROPERTY_NAME_MPP_X);
        const char *pixelSpacingY = openslide_get_property_value(file, OPENSLIDE_PROPERTY_NAME_MPP_Y);
        if(pixelSpacingX != nullptr and pixelSpacingY != nullptr)
            reportInfo() << "WSI pixel spacing: " << pixelSpacingX << ", " << pixelSpacingY << " microns per pixel"
                         << reportEnd();

        //const char * boundsWidth = openslide_get_property_value(file, OPENSLIDE_PROPERTY_NAME_BOUNDS_WIDTH);
        //const char * boundsHeight = openslide_get_property_value(file, OPENSLIDE_PROPERTY_NAME_BOUNDS_HEIGHT);

        //fullWidth = 8000;
        //fullHeight = 8000;
        WholeSlideImageLevel levelData;
        levelData.data = make_uninitialized_unique<uint8_t[]>(fullWidth * fullHeight * 4);
        levelData.width = fullWidth;
        levelData.height = fullHeight;
        std::cout << "Reading " << (fullWidth * fullHeight * 4) / (1024 * 1024) << " MBs" << std::endl;
        mRuntimeManager->startRegularTimer("test");
        openslide_read_region(file, (uint32_t *) levelData.data.get(), 0, 0, level, fullWidth, fullHeight);
        mRuntimeManager->stopRegularTimer("test");
        reportInfo() << "Done reading data" << reportEnd();
        levelList.push_back(levelData);
    }

    mRuntimeManager->printAll();
    // Create image
    auto image = getOutputData<WholeSlideImage>();
    image->create(levelList);

    openslide_close(file);
}

WholeSlideImageImporter::WholeSlideImageImporter() {
    mFilename = "";
    mIsModified = true;
    mGrayscale = false;
    createOutputPort<WholeSlideImage>(0);
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
