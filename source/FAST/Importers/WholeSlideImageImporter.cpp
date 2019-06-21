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
#ifdef WIN32
#else
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

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
    for(int level = 5; level >= 0; --level) {
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
        levelData.width = fullWidth;
        levelData.height = fullHeight;
        float sizeInMB = (fullWidth * fullHeight * 4) / (1024 * 1024);
        std::cout << "Reading " << sizeInMB << " MBs" << std::endl;
        if(sizeInMB < 1000) {
            levelData.data = new uint8_t[fullWidth * fullHeight * 4];
            levelData.memoryMapped = false;
        } else {
            std::cout << "Using memory mapping.." << std::endl;
            uint8_t* data;
            int fd = open(("/tmp/fast_mmap_" + std::to_string(level) + ".bin").c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
            if (fd == -1) {
                perror("Error opening file for writing");
                exit(EXIT_FAILURE);
            }

            /* Stretch the file size to the size of the (mmapped) array of ints
             */
            int result = lseek(fd, fullWidth*fullHeight*4-1, SEEK_SET);
            if (result == -1) {
                close(fd);
                perror("Error calling lseek() to 'stretch' the file");
                exit(EXIT_FAILURE);
            }
            result = write(fd, "", 1);
            if (result != 1) {
                close(fd);
                perror("Error writing last byte of the file");
                exit(EXIT_FAILURE);
            }

            /* Now the file is ready to be mmapped.
             */
            data = (uint8_t*)mmap64(0, fullWidth*fullHeight*4-1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (data == MAP_FAILED) {
                close(fd);
                perror("Error mmapping the file");
                exit(EXIT_FAILURE);
            }
            /*
            if((data = (uint8_t*)mmap64(0, fullWidth*fullHeight*4, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED)
                throw Exception("Error in memory allocation");
                */
            levelData.data = data;
            levelData.memoryMapped = true;
        }
        mRuntimeManager->startRegularTimer("test");
        openslide_read_region(file, (uint32_t *) levelData.data, 0, 0, level, fullWidth, fullHeight);
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
