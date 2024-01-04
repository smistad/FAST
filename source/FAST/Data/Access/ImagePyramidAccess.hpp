#pragma once

#include <FAST/Object.hpp>
#include <FAST/Data/DataTypes.hpp>
#include <unordered_set>
#include <fstream>

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
    LZW, // Lossless compression
    NEURAL_NETWORK, // Use neural network to do the compression and decompression. See ImagePyramid::setCompressionModels
    DEFLATE, // Zlib lossless
};

struct vsi_tile_header {
    uint32_t dummy1;
    uint32_t coord[3];
    uint32_t level;
    uint64_t offset;
    uint32_t numbytes;
    uint32_t dummy2;
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

class FAST_EXPORT ImagePyramidAccess : Object {
public:
	typedef std::unique_ptr<ImagePyramidAccess> pointer;
	ImagePyramidAccess(std::vector<ImagePyramidLevel> levels, openslide_t* fileHandle, TIFF* tiffHandle, std::ifstream* stream, std::vector<vsi_tile_header>& vsiTiles, std::shared_ptr<ImagePyramid> imagePyramid, bool writeAccess, std::unordered_set<std::string>& initializedPatchList, std::mutex& readMutex, ImageCompression compressionFormat);
	void setPatch(int level, int x, int y, std::shared_ptr<Image> patch, bool propagate = true);
	bool isPatchInitialized(uint level, uint x, uint y);
	template <class T>
	std::unique_ptr<T[]> getPatchData(int level, int x, int y, int width, int height);
	std::shared_ptr<Image> getLevelAsImage(int level);
	std::shared_ptr<Image> getPatchAsImage(int level, int offsetX, int offsetY, int width, int height, bool convertToRGB = true);
	std::shared_ptr<Image> getPatchAsImage(int level, int patchIdX, int patchIdY, bool convertToRGB = true);
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
    std::ifstream* m_vsiHandle;
    ImageCompression m_compressionFormat;
    std::vector<vsi_tile_header> m_vsiTiles;
    void readVSITileToBuffer(vsi_tile_header tile, uchar* data);
    uint32_t writeTileToTIFF(int level, int x, int y, std::shared_ptr<Image> image);
    uint32_t writeTileToTIFF(int level, int x, int y, uchar* data, int width, int height, int channels);
    uint32_t writeTileToTIFF(int level, int x, int y, uchar* data);
    uint32_t writeTileToTIFFNeuralNetwork(int level, int x, int y, std::shared_ptr<Image> image);
    int readTileFromTIFF(void* data, int x, int y);
};

template <class T>
std::unique_ptr<T[]> ImagePyramidAccess::getPatchData(int level, int x, int y, int width, int height) {
    auto data = getPatchDataChar(level, x, y, width, height);
    std::unique_ptr<T[]> data2(reinterpret_cast<T*>(data.release())); // Cast to correct type
    return data2;
}

}