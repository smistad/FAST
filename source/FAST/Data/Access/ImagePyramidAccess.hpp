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
 * @sa ImagePyramid
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

/**
 * @brief Object for metadata of an ImagePyramid level
 * @sa ImagePyramid
 * @ingroup wsi
 */
class FAST_EXPORT ImagePyramidLevel {
public:
	int width;
	int height;
	int tileWidth = 1024;
	int tileHeight = 1024;
	int tilesX;
    int tilesY;
	uint64_t offset = 0; // subifd offset used by OME-TIFF
};

/**
 * @brief CPU access to ImagePyramid
 * @ingroup access
 */
#ifdef SWIG
class FAST_EXPORT ImagePyramidAccess {
#else
class FAST_EXPORT ImagePyramidAccess : public Object {
#endif
public:
	typedef std::unique_ptr<ImagePyramidAccess> pointer;
	ImagePyramidAccess(std::vector<ImagePyramidLevel> levels, openslide_t* fileHandle, TIFF* tiffHandle, std::shared_ptr<ImagePyramid> imagePyramid, bool writeAccess, std::unordered_set<std::string>& initializedPatchList, std::mutex& readMutex, ImageCompression compressionFormat, bool useCache = false, int cacheLimit = -1);
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
	/**
	 * @brief Get a specific level in an ImagePyramid as an Image object.
	 * If requesting a level with a width or height higher than 16384 pixels this will throw an exception.
	 * @param level
	 * @return
	 */
	std::shared_ptr<Image> getLevelAsImage(int level);
	/**
	 * @brief Extract a patch from the image pyramid and return it as an Image
	 * @param level Level to extract patch from
	 * @param offsetX X offset
	 * @param offsetY Y offset
	 * @param width Width of patch
	 * @param height Height of patch
	 * @param convertToRGB convert to RGB when using OpenSlide, since it will return BGRA data
	 * @return Patch as Image
	 */
	std::shared_ptr<Image> getPatchAsImage(int level, int offsetX, int offsetY, int width, int height, bool convertToRGB = true);
	/**
	 * @brief Extract a tile from the Image Pyramid
	 * @param level Level to extract tile from
	 * @param patchIdX Tile X id
	 * @param patchIdY  Tile Y id
	 * @param convertToRGB convert to RGB when using OpenSlide, since it will return BGRA data
	 * @return Tile as Image
	 */
	std::shared_ptr<Image> getPatchAsImage(int level, int patchIdX, int patchIdY, bool convertToRGB = true);
	/**
	 * @brief Get patch as Image at a specific magnification
	 * @param magnification Magnification. Example: for 20X this value should be 20
	 * @param offsetX Physical offset x position of patch
	 * @param offsetY Physical offset y position of patch
	 * @param width Width of patch in pixels
	 * @param height Height of patch in pixels
	 * @param convertToRGB convert to RGB when using OpenSlide, since it will return BGRA data
	 * @return patch as Image object
	 */
    std::shared_ptr<Image> getPatchAsImageForMagnification(float magnification, float offsetX, float offsetY, int width, int height, bool convertToRGB = true);
	void release();
	~ImagePyramidAccess() override;
	void setJPEGTables(uint32_t tableCount, void* tableData);
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

    uint32_t m_JPEGTablesCount = 0;
    void* m_JPEGTablesData = nullptr;

    // Optional tile cache
    void addTileToQueue(const std::string& id);
    std::unordered_map<std::string, std::shared_ptr<char[]>> m_tileCache; // Should ideally be a unique_ptr here, but MSVC can't handle that
    std::deque<std::string> m_tileCacheQueue;
    std::unordered_map<std::string, int> m_tileCacheCounter;
    uint64_t m_tileCacheSizeLimit;
    bool m_useTileCache;
};

template <class T>
std::unique_ptr<T[]> ImagePyramidAccess::getPatchData(int level, int x, int y, int width, int height) {
    auto data = getPatchDataChar(level, x, y, width, height);
    std::unique_ptr<T[]> data2(reinterpret_cast<T*>(data.release())); // Cast to correct type
    return data2;
}

}