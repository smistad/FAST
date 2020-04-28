#include "ImagePyramid.hpp"
#include <openslide/openslide.h>
#include <FAST/Utility.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Data/Access/ImagePyramidAccess.hpp>
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

int ImagePyramid::m_counter = 0;

void ImagePyramid::create(int width, int height, int channels, int levels) {
    // TODO Create memory mapped files on disk to use as virtual memory

    if(channels <= 0 || channels > 4)
        throw Exception("Nr of channels must be between 1 and 4");

    // Determine how many levels
    int currentLevel = 0;
    int currentWidth = width;
    int currentHeight = height;
    m_channels = channels;
    while(true) {
		currentWidth = width / std::pow(2, currentLevel);
		currentHeight = height / std::pow(2, currentLevel);

        if(currentWidth < 4096 || currentHeight < 4096)
            break;

        reportInfo() << "Processing level " << currentLevel << reportEnd();
        std::size_t bytes = (std::size_t)currentWidth * currentHeight * m_channels * sizeof(char);

        // Get total size of image
        float sizeInMB = (float)bytes / (1024 * 1024);
        reportInfo() << "WSI level size: " << currentWidth << ", " << currentHeight << ", " << m_channels << reportEnd();
        reportInfo() << "WSI level size: " << sizeInMB << " MBs" << reportEnd();

		ImagePyramidLevel levelData;
		levelData.width = currentWidth;
		levelData.height = currentHeight;

		if(sizeInMB < 512) {
			// If level is less than X MBs, use system memory
			levelData.data = new uint8_t[bytes];
			levelData.memoryMapped = false;
		} else {
			reportInfo() << "Using memory mapping.." << reportEnd();
			uint8_t* data;
#ifdef WIN32
			HANDLE hFile = CreateFile(("C:/windows/temp/fast_mmap_" + std::to_string(currentLevel) + "_" + std::to_string(m_counter) + ".bin").c_str(),
				GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_ALWAYS, NULL, NULL);
			if(hFile == nullptr) {
				throw Exception("Error creating memory mapped file: " + std::to_string(GetLastError()));
			}

			levelData.fileHandle = hFile;

			HANDLE hMapFile = CreateFileMappingA(
				//INVALID_HANDLE_VALUE,    // use paging file, Creating Named Shared Memory
				hFile,
				NULL,                    // default security
				PAGE_READWRITE,          // read/write access
				bytes >> 32,                       // buffer size (high part of 64 bit nr)
				bytes,                // buffer size (low part of 64 bit nr)
				("fast_mmap_" + std::to_string(currentLevel)).c_str());                 // name of mapping object

			if(hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) {
				throw Exception("Error opening memory mapped file");
			}

			data = (uint8_t*) MapViewOfFile(hMapFile,   // handle to map object
				FILE_MAP_ALL_ACCESS, // read/write permission
				0,
				0,
				bytes);
			if(data == nullptr) {
			  throw Exception("Failed to create map view of file: " + std::to_string(GetLastError()));
			}
#else
			int fd = open(("/tmp/fast_mmap_" + std::to_string(currentLevel) + "_" + std::to_string(m_counter) + ".bin").c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
			if (fd == -1) {
				perror("Error opening file for writing");
				exit(EXIT_FAILURE);
			}

			// Stretch the file size to the size of the (mmapped) array of ints
			int result = lseek(fd, bytes-1, SEEK_SET);
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

			// Now the file is ready to be mmapped.
			data = (uint8_t*)mmap64(0, bytes-1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			if (data == MAP_FAILED) {
				close(fd);
				perror("Error mmapping the file");
				exit(EXIT_FAILURE);
			}
			levelData.fileHandle = fd;
#endif
			// Initialize data to all zeros
			std::memset(data, 0, bytes);

			levelData.data = data;
			levelData.memoryMapped = true;
		}
		m_levels.push_back(levelData);

		// TODO Initialize data to zeros?
		reportInfo() << "Done creating level " << currentLevel << reportEnd();
		++currentLevel;
    }

    for(int i = 0; i < m_levels.size(); ++i) {
        int x = m_levels.size() - i - 1;
        //m_levels[i].patches = x*x*x + 10;
		m_levels[i].patches = std::ceil(m_levels[i].width / 256);
    }
    mBoundingBox = BoundingBox(Vector3f(getFullWidth(), getFullHeight(), 0));
    m_initialized = true;
	m_counter += 1;
}

void ImagePyramid::create(openslide_t *fileHandle, std::vector<ImagePyramidLevel> levels) {
    m_fileHandle = fileHandle;
    m_levels = levels;
    m_channels = 4;
    for(int i = 0; i < m_levels.size(); ++i) {
        int x = m_levels.size() - i - 1;
        //m_levels[i].patches = x*x*x + 10;
		m_levels[i].patches = std::ceil(m_levels[i].width / 256);// x* x* x + 10;
    }
    mBoundingBox = BoundingBox(Vector3f(getFullWidth(), getFullHeight(), 0));
    m_initialized = true;
	m_counter += 1;
}

ImagePyramid::ImagePyramid() {
    m_initialized = false;
}

int ImagePyramid::getNrOfLevels() {
    return m_levels.size();
}

int ImagePyramid::getLevelWidth(int level) {
    return m_levels.at(level).width;
}

int ImagePyramid::getLevelHeight(int level) {
    return m_levels.at(level).height;
}

int ImagePyramid::getLevelPatches(int level) {
    return m_levels.at(level).patches;
}

int ImagePyramid::getFullWidth() {
    return m_levels[0].width;
}

int ImagePyramid::getFullHeight() {
    return m_levels[0].height;
}

void ImagePyramid::free(ExecutionDevice::pointer device) {
    freeAll();
}

void ImagePyramid::freeAll() {
    if(m_fileHandle != nullptr) {
        m_levels.clear();
        openslide_close(m_fileHandle);
    } else {
		for(auto& item : m_levels) {
			if(item.memoryMapped) {
#ifdef WIN32
				UnmapViewOfFile(item.data);
				CloseHandle(item.fileHandle);
#else
				munmap(item.data, item.width*item.height*m_channels);
				close(item.fileHandle);
#endif
			} else {
				delete[] item.data;
			}
		}
        m_levels.clear();
    }
	m_initialized = false;
	m_fileHandle = nullptr;
}

ImagePyramid::~ImagePyramid() {
    freeAll();
}
int ImagePyramid::getNrOfChannels() const {
    return m_channels;
}

ImagePyramidAccess::pointer ImagePyramid::getAccess(accessType type) {
    if(!m_initialized)
        throw Exception("ImagePyramid has not been initialized.");

    blockIfBeingWrittenTo();

    if(type == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
        std::unique_lock<std::mutex> lock(mDataIsBeingWrittenToMutex);
        mDataIsBeingWrittenTo = true;
    }
    //updateHostData();
    if(type == ACCESS_READ_WRITE) {
        //setAllDataToOutOfDate();
        updateModifiedTimestamp();
    }
    //mHostDataIsUpToDate = true;
    {
        std::unique_lock<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }
    return std::make_unique<ImagePyramidAccess>(m_levels, m_fileHandle, std::static_pointer_cast<ImagePyramid>(mPtr.lock()), type == ACCESS_READ_WRITE);
}

void ImagePyramid::setDirtyPatch(int level, int patchIdX, int patchIdY) {
	std::lock_guard<std::mutex> lock(m_dirtyPatchMutex);
	const std::string tileString =
		std::to_string(level) + "_" + std::to_string(patchIdX) + "_" + std::to_string(patchIdY);
	m_dirtyPatches.insert(tileString);
}

std::set<std::string> ImagePyramid::getDirtyPatches() {
	std::lock_guard<std::mutex> lock(m_dirtyPatchMutex);
	return m_dirtyPatches;
}

void ImagePyramid::clearDirtyPatches(std::set<std::string> patches) {
	std::lock_guard<std::mutex> lock(m_dirtyPatchMutex);
	for(auto&& patch : patches)
		m_dirtyPatches.erase(patch);
}

}
