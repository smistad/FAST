#include "ImagePyramid.hpp"
#if defined(__APPLE__) || defined(__MACOSX)
#include <openslide.h>
#else
#include <openslide/openslide.h>
#endif
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

#ifndef COMPRESSION_JXL
#define	COMPRESSION_JXL 50002
#endif

namespace fast {

int ImagePyramid::m_counter = 0;

ImagePyramid::ImagePyramid(int width, int height, int channels, int patchWidth, int patchHeight, ImageCompression compression, int compressionQuality, DataType dataType) {
    if(channels <= 0 || channels > 4)
        throw Exception("Nr of channels must be between 1 and 4");

    // Determine how many levels
    int currentLevel = 0;
    int currentWidth = width;
    int currentHeight = height;
    m_channels = channels;
    m_tempFile = true;

    do {
        std::string randomString = generateRandomString(32);
#ifdef WIN32
        m_tiffPath = "C:/windows/temp/fast_image_pyramid_" + randomString + ".tiff";
#else
        m_tiffPath = "/tmp/fast_image_pyramid_" + randomString + ".tiff";
#endif
    } while(fileExists(m_tiffPath));

    TIFFSetErrorHandler([](const char* module, const char* fmt, va_list ap) {
        auto str = make_uninitialized_unique<char[]>(512);
        sprintf(str.get(), fmt, ap);
        std::string str2 = str.get();
        if(strcmp(module, "TIFFAppendToStrip") == 0 && str2.substr(0, 23) == "Write error at scanline") // Suppress error when writing 0 byte patches
            return;
        Reporter::warning() << "TIFF error: " << module << ": " << str.get() << Reporter::end();
    });
    TIFFSetWarningHandler([](const char* module, const char* fmt, va_list ap) {
        auto str = make_uninitialized_unique<char[]>(512);
        sprintf(str.get(), fmt, ap);
        Reporter::warning() << "TIFF warning: " << module << ": " << str.get() << Reporter::end();
    });
    m_tiffHandle = TIFFOpen(m_tiffPath.c_str(), "w8"); // 8 == Bigtiff (64 bit)
    auto tiff = m_tiffHandle;
    m_counter += 1;

    uint photometric;
    uint bitsPerSample = getSizeOfDataType(dataType, 1) * 8;
    m_dataType = dataType;
    uint samplesPerPixel;
    if(channels == 1) {
        photometric = PHOTOMETRIC_MINISBLACK; // Photometric mask causes crash..
        samplesPerPixel = 1;
        if(compression == ImageCompression::UNSPECIFIED)
            compression = ImageCompression::LZW;
    } else {
        if(compression == ImageCompression::UNSPECIFIED)
            compression = ImageCompression::JPEG;
        if(compression == ImageCompression::JPEG) {
            photometric = PHOTOMETRIC_YCBCR; // JPEG is stored using YCBCR internally.
            // If this is not set to YCBCR for JPEG, OpenSlide will not display FAST created WSI tiffs correctly.
        } else {
            photometric = PHOTOMETRIC_RGB;
        }
        samplesPerPixel = 3; // RGBA image pyramid is converted to RGB with getPatchAsImage
    }
    m_compressionFormat = compression;
    m_compressionQuality = compressionQuality;

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
                /*
                TIFFSetField(tiff, TIFFTAG_JPEGQUALITY, m_compressionQuality); // Must be set after previous line // FIXME not working, only 75 gives ok results
                TIFFSetField(tiff, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);
                TIFFSetField(tiff, TIFFTAG_JPEGTABLESMODE, JPEGTABLESMODE_QUANT);
                 */
                break;
            case ImageCompression::JPEGXL:
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_JXL);
                break;
            case ImageCompression::JPEG2000:
                // TODO NOT IMPLEMENTED
                throw NotImplementedException();
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_JP2000);
                break;
            case ImageCompression::NEURAL_NETWORK:
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, 34666); // TODO What should the value be?
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
        TIFFCheckpointDirectory(m_tiffHandle); // Need to check in the tile width and length tags, before writing a tile
        if(m_compressionFormat == ImageCompression::JPEGXL || m_compressionFormat == ImageCompression::JPEG) {
            // TODO Not needed?
            //auto data = std::make_unique<uchar[]>(samplesPerPixel); // Is initialized to zeros
            //auto tileID = TIFFComputeTile(tiff, 0, 0, 0, 0);
            //TIFFSetWriteOffset(tiff, 0); // Set write offset to 0, so that we dont appen data
            //TIFFWriteRawTile(tiff, tileID, data.get(), samplesPerPixel);
        } else {
            auto data = std::make_unique<uchar[]>(levelData.tileWidth*levelData.tileHeight*samplesPerPixel); // Is initialized to zeros
            TIFFWriteTile(tiff, data.get(), 0, 0, 0, 0);
        }
        /*
        // TODO Do we really need to inititalize all tiles? This takes time..
        for(int y = 0; y < levelData.tilesY; ++y) {
            for(int x = 0; x < levelData.tilesX; ++x) {
                TIFFWriteTile(tiff, data.get(), x*levelData.tileWidth, y*levelData.tileHeight, 0, 0);
            }
		}*/
        // END

        TIFFWriteDirectory(m_tiffHandle);
		reportInfo() << "Done creating level " << currentLevel << reportEnd();
		++currentLevel;
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
    auto access =  std::make_unique<ImagePyramidAccess>(m_levels, m_fileHandle, m_tiffHandle, std::static_pointer_cast<ImagePyramid>(mPtr.lock()), type == ACCESS_READ_WRITE, m_initializedPatchList, m_readMutex, m_compressionFormat);
    if(m_JPEGTablesCount > 0)
        access->setJPEGTables(m_JPEGTablesCount, m_JPEGTablesData);
    return access;
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
    TIFFSetDirectory(fileHandle, 0);
    // Get data type
    uint16_t bitsPerSample;
    TIFFGetField(fileHandle, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
    uint16_t sampleFormat;
    TIFFGetField(fileHandle, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
    if(sampleFormat == SAMPLEFORMAT_IEEEFP) {
        if(bitsPerSample == 32) {
            m_dataType = TYPE_FLOAT;
        } else {
            throw Exception("Unsupported TIFF data type, float with " + std::to_string(bitsPerSample) + " bits");
        }
    } else if(sampleFormat == SAMPLEFORMAT_UINT) {
        std::map<uint16_t, DataType> bitsPerSampleMap = {
                {8, TYPE_UINT8},
                {16, TYPE_UINT16},
                };
        if(bitsPerSampleMap.count(bitsPerSample) > 0) {
            m_dataType = bitsPerSampleMap[bitsPerSample];
        } else {
            throw Exception("Unsupported TIFF data type, unsigned integer with " + std::to_string(bitsPerSample) + " bits");
        }
    } else if(sampleFormat == SAMPLEFORMAT_INT) {
        std::map<uint16_t, DataType> bitsPerSampleMap = {
                {8, TYPE_INT8},
                {16, TYPE_INT16},
                };
        if(bitsPerSampleMap.count(bitsPerSample) > 0) {
            m_dataType = bitsPerSampleMap[bitsPerSample];
        } else {
            throw Exception("Unsupported TIFF data type, signed integer with " + std::to_string(bitsPerSample) + " bits");
        }
    } else {
        throw Exception("Unsupported TIFF data type: " + std::to_string(sampleFormat) + " " + std::to_string(bitsPerSample) + " bits");
        //m_dataType = TYPE_UINT8;
    }
    // Get compression
    uint16_t compressionTag;
    TIFFGetField(fileHandle, TIFFTAG_COMPRESSION, &compressionTag);
    ImageCompression compression = ImageCompression::UNSPECIFIED;
    switch(compressionTag) {
        case COMPRESSION_NONE:
            compression = ImageCompression::RAW;
            break;
        case COMPRESSION_JPEG:
            compression = ImageCompression::JPEG;
            {
                int quality = -1;
                int res = TIFFGetField(m_tiffHandle, TIFFTAG_JPEGQUALITY, &quality);
                if(res != 1)
                    throw Exception("Unable to get JPEG quality from TIFF");
                m_compressionQuality = quality;
            }
            {
                // Check if there is JPEG table data stored in the TIFF
                // If so we use this table for all tiles
                uint32_t tablesCount = 0;
                void *tableData;
                int res = TIFFGetField(m_tiffHandle, TIFFTAG_JPEGTABLES, &tablesCount, &tableData);
                if(res == 1) {
                    reportInfo() << "Got JPEG tables data from the TIFF file with size: " << tablesCount << reportEnd();
                    m_JPEGTablesCount = tablesCount;
                    // Have to copy the table
                    m_JPEGTablesData = new uchar[tablesCount];
                    std::memcpy(m_JPEGTablesData, tableData, sizeof(uchar)*tablesCount);
                }
            }
            break;
        case COMPRESSION_JXL:
            compression = ImageCompression::JPEGXL;
            break;
        case COMPRESSION_LZW:
            compression = ImageCompression::LZW;
            break;
        case COMPRESSION_JP2000:
            throw Exception("JPEG 2000 TIFF not supported yet");
            break;
        case COMPRESSION_ADOBE_DEFLATE:
        case COMPRESSION_DEFLATE:
            compression = ImageCompression::DEFLATE;
            break;
        case 34666:
            compression = ImageCompression::NEURAL_NETWORK;
            break;
        default:
            reportWarning() << "Unrecognized compression by FAST in TIFF: " << compressionTag << reportEnd();
    }
    m_compressionFormat = compression;
    // Get spacing from TIFF
    float spacingX;
    float spacingY;
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

int ImagePyramid::getLevelForMagnification(float magnification) {
    if(magnification <= 0)
        throw Exception("Magnification must be larger than 0 in getLevleForMagnification");

    float magnificationLevel0 = getMagnification();
    for(int level = 0; level < m_levels.size(); ++level) {
        float scale = round(getLevelScale(level));
        if(magnificationLevel0 / scale == magnification)
            return level;
    }

    throw Exception("No level corresponded to magnification " + std::to_string(magnification) + " in getLevelForMagnification()");
}

bool ImagePyramid::isOMETIFF() const {
    return m_isOMETIFF;
}

ImageCompression ImagePyramid::getCompression() const {
    return m_compressionFormat;
}

void ImagePyramid::setCompressionModels(std::shared_ptr<NeuralNetwork> compressionModel, std::shared_ptr<NeuralNetwork> decompressionModel, float decompressionOutputScaleFactor) {
    setCompressionModel(compressionModel);
    setDecompressionModel(decompressionModel, decompressionOutputScaleFactor);
}

std::shared_ptr<NeuralNetwork> ImagePyramid::getCompressionModel() const {
    if(!m_compressionModel)
        throw Exception("Image pyramid has no compression model");
    return m_compressionModel;
}

std::shared_ptr<NeuralNetwork> ImagePyramid::getDecompressionModel() const {
    if(!m_decompressionModel)
        throw Exception("Image pyramid has no decompression model");
    return m_decompressionModel;
}

void ImagePyramid::setCompressionModel(std::shared_ptr<NeuralNetwork> compressionModel) {
    m_compressionModel = compressionModel;
}

void ImagePyramid::setDecompressionModel(std::shared_ptr<NeuralNetwork> decompressionModel, float outputScaleFactor) {
    m_decompressionModel = decompressionModel;
    m_decompressionOutputScaleFactor = outputScaleFactor;
}

float ImagePyramid::getDecompressionOutputScaleFactor() const {
    return m_decompressionOutputScaleFactor;
}

DataType ImagePyramid::getDataType() const {
    return m_dataType;
}

int ImagePyramid::getCompressionQuality() const {
    return m_compressionQuality;
}

float ImagePyramid::getMagnification() const {
    if(m_magnification <= 0) {
        // Try to guess magnification from spacing if available
        Vector3f spacing = getSpacing();
        if(spacing != Vector3f::Ones()) {
            // For this calculation we assume the following:
            /*
            * 1 micron = 0.001 millimeters
            * 40X -> 0.00025 mm
            * 20X -> 0.0005 mm
            * 10X -> 0.001 mm
            * 5X -> 0.002 mm
            * 1X -> 0.01 mm
             */
            int magnification = 0;
            if(spacing.x() > 0.00015 && spacing.x() <= 0.00035) {
                magnification = 40;
            } else if(spacing.x() > 0.0004 && spacing.x() < 0.0006) {
                magnification = 20;
            } else if(spacing.x() > 0.00075 && spacing.x() < 0.00125) {
                magnification = 10;
            } else if(spacing.x() > 0.00175 && spacing.x() < 0.00225) {
                magnification = 5;
            } else if(spacing.x() > 0.0075 && spacing.x() < 0.0125) {
                magnification = 1;
            }
            if(magnification == 0) {
                Reporter::info() << "Unable to guess the magnification from the set pixel spacing: " << spacing.x() << " mm" << Reporter::end();
                throw Exception("Magnification of this image pyramid is unknown. You can set it manually using the setMagnification() method");
            } else {
                Reporter::info() << "Guessed the magnification " << magnification << " from the set pixel spacing: " << spacing.x() << " mm" << Reporter::end();
                return magnification;
            }
        } else {
            throw Exception("Magnification of this image pyramid is unknown. You can set it manually using the setMagnification() method");
        }
    }
    return m_magnification;
}

void ImagePyramid::setMagnification(float m) {
    if(m <= 0)
        throw Exception("Magnification given to setMagnification() must be larger than 0");
    m_magnification = m;
}

std::pair<int, float> ImagePyramid::getClosestLevelForMagnification(float magnification, float percentageSlack) {
    // Check if it exists first
    try {
        int level = getLevelForMagnification(magnification);
        return {level, 1.0f};
    } catch(Exception &e) {

    }

    float level0spacing = getSpacing().x();
    if(getSpacing() == Vector3f::Ones() && m_magnification > 0) {
        level0spacing = 0.00025f * (40.0f / (float)m_magnification);
    }

    // First find level which is larger than request magnification
    float targetSpacing = 0.00025f * (40.0f / (float)magnification);
    int level = 0;
    float resampleFactor = 1.0f;
    for(int i = 0; i < getNrOfLevels(); ++i) {
        float levelSpacing = getLevelScale(i)*level0spacing;
        level = i;
        resampleFactor = targetSpacing / levelSpacing; // Scale between level and the magnification level we want
        if(i+1 < getNrOfLevels() && getLevelScale(i+1)*level0spacing > targetSpacing) {
            break;
        }
    }
    if(level < 0)
        throw Exception("Unable to get closest magnification level to " +
        std::to_string(magnification) + " because level 0 was at a lower magnification ");
    if(std::fabs(resampleFactor - 1.0f) < percentageSlack) { // If within 1.1 - 0.9 resampleFactor, we just sample from the specific level
        resampleFactor = 1.0f;
    }
    return {level, resampleFactor};
}

}
