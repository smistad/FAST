#pragma once

#include <FAST/Object.hpp>
#include <FAST/Data/DataTypes.hpp>
#include <unordered_set>

// Forward declare
typedef struct _openslide openslide_t;
typedef struct tiff TIFF;

namespace fast {

class Image;
class ImagePyramid;
class NeuralNetwork;


/**
 * @brief Image compression types for ImagePyramids
 *
 * @ingroup wsi
 */
enum class ImageCompression {
    UNSPECIFIED,
    RAW,
    JPEG,
    JPEG2000,
    JPEGXL,
    LZW, // Lossless compression
    NEURAL_NETWORK, // Use neural network to do the compression and decompression. See ImagePyramid::setCompressionModels
    DEFLATE, // Zlib lossless
};

class FAST_EXPORT ImagePyramidPatch {
public:
	std::unique_ptr<uchar[]> data;
	int width;
	int height;
	int offsetX;
	int offsetY;
};

class FAST_EXPORT ImagePyramidLevel {
public:
	int width;
	int height;
	int tileWidth = 256;
	int tileHeight = 256;
	int tilesX;
    int tilesY;
	bool memoryMapped;
	uint8_t* data;
	uint64_t offset = 0; // subifd offset used by OME-TIFF
#ifdef WIN32
	void* fileHandle;
#else
	int fileHandle;
#endif
};

/**
 * @brief CPU access to ImagePyramid
 * @ingroup access
 */
class FAST_EXPORT ImagePyramidAccess : Object {
public:
	typedef std::unique_ptr<ImagePyramidAccess> pointer;
	ImagePyramidAccess(std::vector<ImagePyramidLevel> levels, openslide_t* fileHandle, TIFF* tiffHandle, std::shared_ptr<ImagePyramid> imagePyramid, bool writeAccess, std::unordered_set<std::string>& initializedPatchList, std::mutex& readMutex, ImageCompression compressionFormat);
	/**
	 * @brief Write a patch to the pyramid
	 * @param level
	 * @param x
	 * @param y
	 * @param patch
	 * @param propagate
	 */
	void setPatch(int level, int x, int y, std::shared_ptr<Image> patch, bool propagate = true);
	/**
	 * @brief Write patch/tile as empty. It will render as white/black
	 * @param level
	 * @param x
	 * @param y
	 */
	void setBlankPatch(int level, int x, int y);
	bool isPatchInitialized(int level, int x, int y);
	template <class T>
	std::unique_ptr<T[]> getPatchData(int level, int x, int y, int width, int height);
	std::shared_ptr<Image> getLevelAsImage(int level);
	std::shared_ptr<Image> getPatchAsImage(int level, int offsetX, int offsetY, int width, int height, bool convertToRGB = true);
	std::shared_ptr<Image> getPatchAsImage(int level, int patchIdX, int patchIdY, bool convertToRGB = true);
	/**
	 * @brief Get patch as Image at a specific magnification
	 * @param magnification Magnification. Example: for 20X this value should be 20
	 * @param offsetX Physical offset x position of patch
	 * @param offsetY Physical offset y position of patch
	 * @param width Width of patch in pixels
	 * @param height Height of patch in pixels
	 * @param convertToRGB Convert from BGR to RGB if needed
	 * @return patch as Image object
	 */
    std::shared_ptr<Image> getPatchAsImageForMagnification(float magnification, float offsetX, float offsetY, int width, int height, bool convertToRGB = true);
	void release();
	~ImagePyramidAccess();
private:
    std::unique_ptr<uchar[]> getPatchDataChar(int level, int x, int y, int width, int height);
	std::shared_ptr<ImagePyramid> m_image;
	std::vector<ImagePyramidLevel> m_levels;
	bool m_write;
	openslide_t* m_fileHandle = nullptr;
	TIFF* m_tiffHandle = nullptr;
    std::unordered_set<std::string>& m_initializedPatchList; // Keep a list of initialized patches, for tiff backend
    std::mutex& m_readMutex;
    ImageCompression m_compressionFormat;
    uint32_t writeTileToTIFF(int level, int x, int y, std::shared_ptr<Image> image);
    uint32_t writeTileToTIFF(int level, int x, int y, uchar* data, int width, int height, int channels);
    uint32_t writeTileToTIFF(int level, int x, int y, uchar* data);
    uint32_t writeTileToTIFFJPEGXL(int level, int x, int y, uchar *data);
    uint32_t writeTileToTIFFJPEG(int level, int x, int y, uchar *data);
    uint32_t writeTileToTIFFNeuralNetwork(int level, int x, int y, std::shared_ptr<Image> image);
    int readTileFromTIFF(void* data, int x, int y, int level);
    void propagatePatch(std::shared_ptr<Image> patch, int level, int x, int y);
};

template <class T>
std::unique_ptr<T[]> ImagePyramidAccess::getPatchData(int level, int x, int y, int width, int height) {
    auto data = getPatchDataChar(level, x, y, width, height);
    std::unique_ptr<T[]> data2(reinterpret_cast<T*>(data.release())); // Cast to correct type
    return data2;
}

}