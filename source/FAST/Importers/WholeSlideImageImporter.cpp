#include "WholeSlideImageImporter.hpp"
#include <memory>
#include "FAST/Data/DataTypes.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Data/Image.hpp"
#include <cctype>
#include <algorithm>
#include <openslide/openslide.h>
#include <FAST/Data/WholeSlideImage.hpp>
#ifdef WIN32
#include <winbase.h>
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

    if(!fileExists(mFilename))
        throw FileNotFoundException(mFilename);

    openslide_t* file = openslide_open(mFilename.c_str());
    if(file == nullptr) {
        const char * message = openslide_get_error(file);
        throw Exception("Unable to open file " + mFilename + ". OpenSlide error message: " + message);
    }

    auto image = getOutputData<WholeSlideImage>();

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

    std::vector<WholeSlideImageLevel> levelList;
    int levels = openslide_get_level_count(file);
    reportInfo() << "WSI has " << levels << " levels" << reportEnd();
    for(int level = levels-1; level >= 0; --level) {
        reportInfo() << "Processing level " << level << reportEnd();
        // Get total size of image
        int64_t fullWidth = -1, fullHeight = -1;
        openslide_get_level_dimensions(file, level, &fullWidth, &fullHeight); // Level 0 is the largest level    
        std::size_t bytes = fullWidth * fullHeight * 4;
        float sizeInMB = bytes / (1024 * 1024);
        reportInfo() << "WSI level size: " << fullWidth << ", " << fullHeight << reportEnd();
        reportInfo() << "WSI level size: " << sizeInMB << " MBs" << reportEnd();
        if(sizeInMB <= 4) {
            reportInfo() << "WSI level was less than 4 MB, skipping.." << reportEnd();
            continue;
        }

        const char *pixelSpacingX = openslide_get_property_value(file, OPENSLIDE_PROPERTY_NAME_MPP_X);
        const char *pixelSpacingY = openslide_get_property_value(file, OPENSLIDE_PROPERTY_NAME_MPP_Y);
        if(pixelSpacingX != nullptr && pixelSpacingY != nullptr)
            reportInfo() << "WSI pixel spacing: " << pixelSpacingX << ", " << pixelSpacingY << " microns per pixel"
                         << reportEnd();

        WholeSlideImageLevel levelData;
        levelData.width = fullWidth;
        levelData.height = fullHeight;

        if(sizeInMB < 1000) {
            levelData.data = new uint8_t[bytes];
            levelData.memoryMapped = false;
        } else {
            reportInfo() << "Using memory mapping.." << reportEnd();
            uint8_t* data;
#ifdef WIN32
            HANDLE hFile = CreateFile(("C:/windows/temp/fast_mmap_" + std::to_string(level) + ".bin").c_str(), 
                GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE, 
                NULL, OPEN_ALWAYS, NULL, NULL);
            levelData.fileHandle = hFile;

            HANDLE hMapFile = CreateFileMappingA(
                //INVALID_HANDLE_VALUE,    // use paging file, Creating Named Shared Memory
                hFile,
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                bytes >> 32,                       // buffer size (high part of 64 bit nr)
                bytes,                // buffer size (low part of 64 bit nr)
                ("fast_mmap_" + std::to_string(level)).c_str());                 // name of mapping object

                if(hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) {
                    throw Exception("Error opening memory mapped file");
                }

                data = (uint8_t*) MapViewOfFile(hMapFile,   // handle to map object
                    FILE_MAP_ALL_ACCESS, // read/write permission
                    0,
                    0,
                    bytes);
            if(data == nullptr) {
              throw Exception("Failed to create map view of file" + std::to_string(GetLastError()));
            }
#else
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
            levelData.fileHandle = (void*)fd;
#endif
            levelData.data = data;
            levelData.memoryMapped = true;
        }
        openslide_read_region(file, (uint32_t *) levelData.data, 0, 0, level, fullWidth, fullHeight);
        reportInfo() << "Done reading data for level " << level << reportEnd();
        levelList.push_back(levelData);
    }

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
