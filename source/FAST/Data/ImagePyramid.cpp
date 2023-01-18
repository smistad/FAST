#include "ImagePyramid.hpp"
#include <openslide/openslide.h>
#include <tiffio.h>
#include <FAST/Utility.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Data/Access/ImagePyramidAccess.hpp>
#include <utility>
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

ImagePyramid::ImagePyramid(int width, int height, int channels, int patchWidth, int patchHeight) {
    if(channels <= 0 || channels > 4)
        throw Exception("Nr of channels must be between 1 and 4");

    // Determine how many levels
    int currentLevel = 0;
    int currentWidth = width;
    int currentHeight = height;
    m_channels = channels;
    m_tempFile = true;

    std::cout << "Creating tiff path.." << std::endl;
    do {
        std::string randomString = generateRandomString(32);
        std::cout << "random string: " << randomString << std::endl;
#ifdef WIN32
        m_tiffPath = "C:/windows/temp/fast_image_pyramid_" + randomString + ".tiff";
#else
        m_tiffPath = "/tmp/fast_image_pyramid_" + randomString + ".tiff";
#endif
        std::cout << "TIFF path: " << m_tiffPath << std::endl;
    } while(fileExists(m_tiffPath));

    TIFFSetErrorHandler([](const char* module, const char* fmt, va_list ap) {
        auto str = make_uninitialized_unique<char[]>(512);
        sprintf(str.get(), fmt, ap);
        Reporter::warning() << "TIFF: " << module << ": " << str.get() << Reporter::end();
    });
    TIFFSetWarningHandler([](const char* module, const char* fmt, va_list ap) {
        auto str = make_uninitialized_unique<char[]>(512);
        sprintf(str.get(), fmt, ap);
        Reporter::warning() << "TIFF: " << module << ": " << str.get() << Reporter::end();
    });
    m_tiffHandle = TIFFOpen(m_tiffPath.c_str(), "w8");
    auto tiff = m_tiffHandle;
    m_counter += 1;

    ImageCompression compression = ImageCompression::LZW;

    uint photometric = PHOTOMETRIC_RGB;
    uint bitsPerSample = 8;
    uint samplesPerPixel = 3; // RGBA image pyramid is converted to RGB with getPatchAsImage
    if(channels == 1) {
        photometric = PHOTOMETRIC_MINISBLACK; // Photometric mask causes crash..
        samplesPerPixel = 1;
    }

    while(true) {
		currentWidth = width / std::pow(2, currentLevel);
		currentHeight = height / std::pow(2, currentLevel);

        if(currentLevel > 0 && (currentWidth < 4096 && currentHeight < 4096)) // IMPORTANT: This should be the same as in PatchStitcher.
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
		levelData.tileWidth = patchWidth;
        levelData.tileHeight = patchHeight;
        levelData.tilesX = std::ceil((float)levelData.width / levelData.tileWidth);
        levelData.tilesY = std::ceil((float)levelData.height / levelData.tileHeight);

        // Write base tags
        TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, photometric);
        TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
        TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
        TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

        if(currentLevel > 0) {
            // All levels except highest res level should have this tag?
            TIFFSetField(tiff, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE);
        }
        switch(compression) {
            case ImageCompression::RAW:
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
                break;
            case ImageCompression::LZW:
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
                break;
            case ImageCompression::JPEG:
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
                break;
            case ImageCompression::JPEG2000:
                // TODO NOT IMPLEMENTED
                throw NotImplementedException();
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_JP2000);
                break;
        }

        TIFFSetField(tiff, TIFFTAG_TILEWIDTH, levelData.tileWidth);
        TIFFSetField(tiff, TIFFTAG_TILELENGTH, levelData.tileHeight);
        TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, levelData.width);
        TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, levelData.height);

		m_levels.push_back(levelData);

        // TODO need to initialize somehow?
		// We need to write the first tile for some reason... or we will get an error saying it is missing required
		// TileOffsets
		auto data = std::make_unique<uchar[]>(levelData.tileWidth*levelData.tileHeight*samplesPerPixel); // Is initialized to zeros
        TIFFWriteTile(tiff, data.get(), 0, 0, 0, 0);
        /*
        // TODO Do we really need to inititalize all tiles? This takes time..
        for(int y = 0; y < levelData.tilesY; ++y) {
            for(int x = 0; x < levelData.tilesX; ++x) {
                TIFFWriteTile(tiff, data.get(), x*levelData.tileWidth, y*levelData.tileHeight, 0, 0);
            }
		}*/
        // END

		reportInfo() << "Done creating level " << currentLevel << reportEnd();
		++currentLevel;
		TIFFWriteDirectory(m_tiffHandle);
    }

    mBoundingBox = DataBoundingBox(Vector3f(getFullWidth(), getFullHeight(), 0));
    m_initialized = true;
    m_pyramidFullyInitialized = false;
	m_counter += 1;
}

ImagePyramid::ImagePyramid(openslide_t *fileHandle, std::vector<ImagePyramidLevel> levels) {
    m_fileHandle = fileHandle;
    m_levels = std::move(levels);
    m_channels = 4;
    for(int i = 0; i < m_levels.size(); ++i) {
		m_levels[i].tilesX = std::ceil((float)m_levels[i].width / m_levels[i].tileWidth);
        m_levels[i].tilesY = std::ceil((float)m_levels[i].height / m_levels[i].tileHeight);
    }
    mBoundingBox = DataBoundingBox(Vector3f(getFullWidth(), getFullHeight(), 0));
    m_initialized = true;
    m_pyramidFullyInitialized = true;
	m_counter += 1;
}
ImagePyramid::ImagePyramid(std::ifstream* stream, std::vector<vsi_tile_header> tileHeaders, std::vector<ImagePyramidLevel> levels, ImageCompression compressionFormat) {
    m_vsiFileHandle = stream;
    m_levels = std::move(levels);
    m_vsiTiles = std::move(tileHeaders);
    m_channels = 3;
    mBoundingBox = DataBoundingBox(Vector3f(getFullWidth(), getFullHeight(), 0));
    m_initialized = true;
    m_pyramidFullyInitialized = true;
    m_compressionFormat = compressionFormat;
}

ImagePyramid::ImagePyramid() {
    m_initialized = false;
    m_pyramidFullyInitialized = false;
}

ImagePyramidLevel ImagePyramid::getLevelInfo(int level) {
    /*if(!m_initialized)
        throw Exception("ImagePyramid has not been initialized.");*/ // TODO why does this fail?
    if(level < 0) // Negative level means last level (lowest resolution)
        level = getNrOfLevels()-1;
    if(level < 0 || level > m_levels.size()-1)
        throw Exception("Level " + std::to_string(level) + " doesn't exist in ImagePyramid. Pyramid has " + std::to_string(m_levels.size()) + " levels");
    return m_levels[level];
}

int ImagePyramid::getNrOfLevels() {
    return m_levels.size();
}

int ImagePyramid::getLevelWidth(int level) {
    return getLevelInfo(level).width;
}

int ImagePyramid::getLevelHeight(int level) {
    return getLevelInfo(level).height;
}

int ImagePyramid::getLevelTileWidth(int level) {
    return getLevelInfo(level).tileWidth;
}

int ImagePyramid::getLevelTileHeight(int level) {
    return getLevelInfo(level).tileHeight;
}

int ImagePyramid::getLevelTilesX(int level) {
    return getLevelInfo(level).tilesX;
}

int ImagePyramid::getLevelTilesY(int level) {
    return getLevelInfo(level).tilesY;
}

int ImagePyramid::getFullWidth() {
    return getLevelInfo(0).width;
}

int ImagePyramid::getFullHeight() {
    return getLevelInfo(0).height;
}

void ImagePyramid::free(ExecutionDevice::pointer device) {
    freeAll();
}

void ImagePyramid::freeAll() {
    if(m_fileHandle != nullptr) {
        m_levels.clear();
        openslide_close(m_fileHandle);
    } else if(m_tiffHandle != nullptr) {
        m_levels.clear();
        TIFFClose(m_tiffHandle);
        if(m_tempFile) {
            // If this is a temp file created by FAST. Delete it.
            std::remove(m_tiffPath.c_str());
        }
    } else if(!m_vsiTiles.empty()) {
        m_vsiFileHandle->close();
        delete m_vsiFileHandle;
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
    return std::make_unique<ImagePyramidAccess>(m_levels, m_fileHandle, m_tiffHandle, m_vsiFileHandle, m_vsiTiles, std::static_pointer_cast<ImagePyramid>(mPtr.lock()), type == ACCESS_READ_WRITE, m_initializedPatchList, m_readMutex, m_compressionFormat);
}

void ImagePyramid::setDirtyPatch(int level, int patchIdX, int patchIdY) {
	std::lock_guard<std::mutex> lock(m_dirtyPatchMutex);
	const std::string tileString =
		std::to_string(level) + "_" + std::to_string(patchIdX) + "_" + std::to_string(patchIdY);
	m_dirtyPatches.insert(tileString);
}

std::unordered_set<std::string> ImagePyramid::getDirtyPatches() {
	std::lock_guard<std::mutex> lock(m_dirtyPatchMutex);
	return m_dirtyPatches;
}

bool ImagePyramid::isDirtyPatch(const std::string& tileID) {
	std::lock_guard<std::mutex> lock(m_dirtyPatchMutex);
	return m_dirtyPatches.count(tileID);
}

void ImagePyramid::clearDirtyPatches(std::set<std::string> patches) {
	std::lock_guard<std::mutex> lock(m_dirtyPatchMutex);
	for(auto&& patch : patches)
		m_dirtyPatches.erase(patch);
}

void ImagePyramid::setSpacing(Vector3f spacing) {
	m_spacing = spacing;
    if(m_tiffHandle != nullptr) {
        // Write spacing to TIFF file
		if(spacing.x() != 1 && spacing.y() != 1) { // Spacing == 1 means not set.
            auto access = getAccess(ACCESS_READ_WRITE); // Ensure we have exclusive access to TIFF
            for(int level = 0; level < getNrOfLevels(); ++level) {
                TIFFSetDirectory(m_tiffHandle, level);
                TIFFSetField(m_tiffHandle, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER);
                float scaleX = (float)getFullWidth() / getLevelWidth(level);
                float scaleY = (float)getFullHeight() / getLevelHeight(level);
                TIFFSetField(m_tiffHandle, TIFFTAG_XRESOLUTION,
                    1.0f / (spacing.x() / 10.0f) * scaleX); // Convert to cm, and adjust for level
                TIFFSetField(m_tiffHandle, TIFFTAG_YRESOLUTION,
                    1.0f / (spacing.y() / 10.0f) * scaleY); // Convert to cm, and adjust for level
                TIFFRewriteDirectory(m_tiffHandle); // Write changes
            }
        }
    }
}

Vector3f ImagePyramid::getSpacing() const {
	return m_spacing;
}

ImagePyramid::ImagePyramid(TIFF *fileHandle, std::vector<ImagePyramidLevel> levels, int channels, bool isOMETIFF) {
    m_isOMETIFF = isOMETIFF;
    if(channels <= 0 || channels > 4)
        throw Exception("Nr of channels must be between 1 and 4 in ImagePyramid when importing from TIFF");
    m_tiffHandle = fileHandle;
    m_levels = levels;
    m_channels = channels;
    for(int i = 0; i < m_levels.size(); ++i) {
        m_levels[i].tilesX = std::ceil((float)m_levels[i].width / m_levels[i].tileWidth);
        m_levels[i].tilesY = std::ceil((float)m_levels[i].height / m_levels[i].tileHeight);
    }
    // Get spacing from TIFF
    float spacingX;
    float spacingY;
    TIFFSetDirectory(fileHandle, 0);
    int resX = TIFFGetField(fileHandle, TIFFTAG_XRESOLUTION, &spacingX);
    int resY = TIFFGetField(fileHandle, TIFFTAG_YRESOLUTION, &spacingY);
    if(resX == 1 && resY == 1) {
        // Convert from cm
        spacingX = 1.0f/(spacingX/10.0f);
        spacingY = 1.0f/(spacingY/10.0f);
        reportInfo() << "Spacing from TIFF was " << spacingX << " " << spacingY << reportEnd();
        m_spacing = Vector3f(spacingX, spacingY, 1.0f);
    }
    mBoundingBox = DataBoundingBox(Vector3f(getFullWidth(), getFullHeight(), 0));
    m_initialized = true;
    m_pyramidFullyInitialized = true;
    m_counter += 1;
}

bool ImagePyramid::isBGRA() const {
    return m_fileHandle != nullptr;
}

bool ImagePyramid::usesTIFF() const {
    return m_tiffHandle != nullptr;
}

std::string ImagePyramid::getTIFFPath() const {
    return m_tiffPath;
}

bool ImagePyramid::usesOpenSlide() const {
    return m_fileHandle != nullptr;
}

bool ImagePyramid::isPyramidFullyInitialized() const {
    return m_pyramidFullyInitialized;
}

float ImagePyramid::getLevelScale(int level) {
    return (float)getFullWidth()/getLevelWidth(level);
}


DataBoundingBox ImagePyramid::getTransformedBoundingBox() const {
    auto T = SceneGraph::getEigenTransformFromNode(getSceneGraphNode());

    // Add image spacing
    T.scale(getSpacing());

    return SpatialDataObject::getBoundingBox().getTransformedBoundingBox(T);
}

DataBoundingBox ImagePyramid::getBoundingBox() const {
    // Add image spacing
    auto T = Affine3f::Identity();
    T.scale(getSpacing());

    return SpatialDataObject::getBoundingBox().getTransformedBoundingBox(T);
}

int ImagePyramid::getLevelForMagnification(int magnification, float slackPercentage) {
    if(magnification <= 0)
        throw Exception("Magnification must be larger than 0 in getLevleForMagnification");
    const Vector3f spacing = getSpacing();
    // For this calculation we assume the following:
    /*
    * 1 micron = 0.001 millimeters
    * 40X -> 0.00025 mm
    * 20X -> 0.0005 mm
    * 10X -> 0.001 mm
    * 5X -> 0.002 mm
    * 1X -> 0.01 mm
     */
    float level0spacing = spacing.x();
    if(!m_vsiTiles.empty()) {
        // For VSI format we assume that level 0 is 40X for now.
        // Because we have now spacing information for this format.
        level0spacing = 0.00025;
        reportWarning() << "Assuming the image pyramid is 40X since not able to extract pixel spacing information from the Olympus CellSense format atm." << reportEnd();
    }
    float targetSpacing = 0.00025f * (40.0f / (float)magnification);
    float minDistance = std::numeric_limits<float>::max();
    int levelResult = 0;
    for(int i = 0; i < m_levels.size(); ++i) {
        auto scale = getLevelScale(i);
        float levelSpacing = scale*level0spacing;
        float distance = std::fabs(levelSpacing - targetSpacing);
        if(distance < minDistance) {
            minDistance = distance;
            levelResult = i;
        }
    }

    // How much slack to allow?
    if(minDistance > targetSpacing*slackPercentage)
        throw Exception("No level close enough to magnification of " + std::to_string(magnification) + " was found in the image pyramid.");

    return levelResult;
}

bool ImagePyramid::isOMETIFF() const {
    return m_isOMETIFF;
}

}
