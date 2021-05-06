#include "ImagePyramidAccess.hpp"
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Algorithms/ImageChannelConverter/ImageChannelConverter.hpp>
#include <FAST/Utility.hpp>
#include <openslide/openslide.h>
#include <tiffio.h>
#include <FAST/Data/Image.hpp>

namespace fast {

ImagePyramidAccess::ImagePyramidAccess(std::vector<ImagePyramidLevel> levels, openslide_t* fileHandle, TIFF* tiffHandle, std::shared_ptr<ImagePyramid> imagePyramid, bool write) {
	if(levels.size() == 0)
		throw Exception("Image pyramid has no levels");
	m_image = imagePyramid;
	m_levels = levels;
	m_write = write;
    m_fileHandle = fileHandle;
    m_tiffHandle = tiffHandle;
}

void ImagePyramidAccess::release() {
	m_image->accessFinished();
}

ImagePyramidAccess::~ImagePyramidAccess() {
	release();
}

void ImagePyramidAccess::setScalarFast(uint x, uint y, uint level, uint8_t value, uint channel) noexcept {
    if(!m_write)
        return;
	auto levelData = m_levels[level];
    std::size_t pos = (x + (std::size_t)y * levelData.width) * m_image->getNrOfChannels() + channel;
	levelData.data[pos] = value;

    // add patch to list of dirty patches
    int levelWidth = m_image->getLevelWidth(level);
    int levelHeight = m_image->getLevelHeight(level);
    const int tilesX = m_image->getLevelTilesX(level);
    const int tilesY = m_image->getLevelTilesY(level);
    int patchIdX = std::floor(((float)x / levelWidth)* tilesX);
    int patchIdY = std::floor(((float)y / levelHeight)* tilesY);
    m_image->setDirtyPatch(level, patchIdX, patchIdY);

    // Propagate change upwards recursively
    if(level != m_levels.size() - 1) {
        int nx = x / 2;
        int ny = y / 2;
        // Calculate new average:
        float sum = getScalarFast(nx*2, ny*2, level, channel);
        int counter = 1;
        if(nx * 2 + 1 < levelWidth) {
            sum += getScalarFast(nx * 2 + 1, ny * 2, level, channel);
            ++counter;
        }
        if(ny * 2 + 1 < levelHeight) {
            sum += getScalarFast(nx * 2, ny * 2 + 1, level, channel);
            ++counter;
        }
        if(nx * 2 + 1 < levelWidth && ny * 2 + 1 < levelHeight) {
            sum += getScalarFast(nx * 2 + 1, ny * 2 + 1, level, channel);
            ++counter;
        }
        setScalarFast(nx, ny, level + 1, std::round(sum / counter), channel);
    }
}

void ImagePyramidAccess::setScalar(uint x, uint y, uint level, uint8_t value, uint channel) {
	// Make sure it has write rights
	if(!m_write)
		throw Exception("ImagePyramidAccess has not write rights, but tried to write a value");
	auto levelData = m_levels[level];
	if(x >= levelData.width || y >= levelData.height)
		throw OutOfBoundsException();

    setScalarFast(x, y, level, value, channel);
}

uint8_t ImagePyramidAccess::getScalar(uint x, uint y, uint level, uint channel) {
	auto levelData = m_levels[level];
	if(x >= levelData.width || y >= levelData.height)
		throw OutOfBoundsException();
	return levelData.data[(x + y * levelData.width) * m_image->getNrOfChannels() + channel];
}

uint8_t ImagePyramidAccess::getScalarFast(uint x, uint y, uint level, uint channel) noexcept {
	auto levelData = m_levels[level];
	return levelData.data[(x + y * levelData.width) * m_image->getNrOfChannels() + channel];
}


ImagePyramidPatch ImagePyramidAccess::getPatch(std::string tile) {
    auto parts = split(tile, "_");
    if(parts.size() != 3)
        throw Exception("incorrect tile format");

    int level = std::stoi(parts[0]);
    int tile_x = std::stoi(parts[1]);
    int tile_y = std::stoi(parts[2]);

    return getPatch(level, tile_x, tile_y);
}

std::unique_ptr<uchar[]> ImagePyramidAccess::getPatchData(int level, int x, int y, int width, int height) {
    const int levelWidth = m_image->getLevelWidth(level);
    const int levelHeight = m_image->getLevelHeight(level);
    const int channels = m_image->getNrOfChannels();
    auto data = std::make_unique<uchar[]>(width*height*channels);
    if(m_tiffHandle != nullptr) {
        // Read all tiles within region, then crop
        const int tileWidth = m_image->getLevelTileWidth(level);
        const int tileHeight = m_image->getLevelTileHeight(level);
        TIFFSetDirectory(m_tiffHandle, level); // TODO Only set if needed?
        if(width == tileWidth && height == tileHeight) {
            int bytesRead = TIFFReadTile(m_tiffHandle, (void *) data.get(), x, y, 0, 0);
        } else if(width < tileWidth || height < tileHeight) {
            // In TIFF all tiles have the same size, thus they are padded..
            auto tileData = std::make_unique<uchar[]>(tileWidth*tileHeight*channels);
            int bytesRead = TIFFReadTile(m_tiffHandle, (void *)tileData.get(), x, y, 0, 0);
            // Remove padding
            for(int dy = 0; dy < height; ++dy) {
                for(int dx = 0; dx < width; ++dx) {
                    data[dx + dy*width] = tileData[dx + dy*tileWidth];
                }
            }
        } else if(width > tileWidth || height > tileHeight) {
            // TODO implement
            throw Exception("Encountered unqueal tile and request size. Not implemented. "
            + std::to_string(width) + " " + std::to_string(tileWidth) + " "
            + std::to_string(height) + " " + std::to_string(tileHeight)
            );
            throw NotImplementedException();
            // TODO how to do this fast?
            /*
            int startTileX =;
            int startTileY =;
            int endTileX =;
            int endTileY =;
            auto dataBuffer = std::make_unique<char[]>(tileWidth * tileHeight * channels);
            int offsetX =;
            int offsetY =;
            for (int x = startTileX; x <= endTileX; ++x) {
                for (int y = startTileY; y <= endTileY; ++y) {
                    TIFFReadTile(m_tiffHandle, (void *) dataBuffer.get(), x, y, 0, 0);
                    for (int)
                }
            }
             */
        }

    } else if(m_fileHandle != nullptr) {
		int scale = (float)m_image->getFullWidth()/levelWidth;
#ifndef WIN32
        // HACK for black edge frames on ubuntu linux 20.04. This seems to be an issue with openslide or underlying libraries
		if(level != 0) { // only occurs on levels != 0
		    // Reading scale pixels further in eliminates the problem for some reason...
		    x = x == 0 ? x+1 : x;
            y = y == 0 ? y+1 : y;
		}
#endif
        openslide_read_region(m_fileHandle, (uint32_t*)data.get(), x * scale, y * scale, level, width, height);
    } else {
        auto levelData = m_levels[level];
        for(int cy = y; cy < std::min(y + height, levelHeight); ++cy) {
            for(int cx = x; cx < std::min(x + width, levelWidth); ++cx) {
                for(int channel = 0; channel < channels; ++channel) {
                    data[(cx - x + (cy - y) * width)*channels + channel] = levelData.data[(cx + cy * levelWidth)*channels + channel];
                }
            }
        }
    }

    return data;
}

ImagePyramidPatch ImagePyramidAccess::getPatch(int level, int tile_x, int tile_y) {
    // Create patch
    int levelWidth = m_image->getLevelWidth(level);
    int levelHeight = m_image->getLevelHeight(level);
    int levelTileWidth = m_image->getLevelTileWidth(level);
    int levelTileHeight = m_image->getLevelTileHeight(level);
    int tilesX = m_image->getLevelTilesX(level);
    int tilesY = m_image->getLevelTilesY(level);
    ImagePyramidPatch tile;
    tile.offsetX = tile_x * levelTileWidth;
    tile.offsetY = tile_y * levelTileHeight;

    tile.width = levelTileWidth;
    if(tile_x == tilesX - 1)
        tile.width = levelWidth - tile.offsetX;
    tile.height = levelTileHeight;
    if(tile_y == tilesY - 1)
        tile.height = levelHeight - tile.offsetY;

    // Read the actual data
    tile.data = getPatchData(level, tile.offsetX, tile.offsetY, tile.width, tile.height);

    return tile;
}

std::shared_ptr<Image> ImagePyramidAccess::getLevelAsImage(int level) {
    if(level < 0 || level >= m_image->getNrOfLevels())
        throw Exception("Incorrect level given to getLevelAsImage" + std::to_string(level));

    int width = m_image->getLevelWidth(level);
    int height = m_image->getLevelHeight(level);
    if(width > 16384 || height > 16384)
        throw Exception("Image level is too large to convert into a FAST image");

    auto image = Image::New();
    if(m_fileHandle == nullptr) {
		image->create(width, height, TYPE_UINT8, 4, m_levels[level].data);
    } else {
		auto data = make_uninitialized_unique<uchar[]>(width*height*4);
        openslide_read_region(m_fileHandle, (uint32_t*)data.get(), 0, 0, level, width, height);
		image->create(width, height, TYPE_UINT8, 4, std::move(data));
    }
    image->setSpacing(Vector3f(
            (float)m_image->getFullWidth() / width,
            (float)m_image->getFullHeight() / height,
            1.0f
    ));
    SceneGraph::setParentNode(image, std::dynamic_pointer_cast<SpatialDataObject>(m_image));

    // Data is stored as BGRA, need to delete alpha channel and reverse it
    auto channelConverter = ImageChannelConverter::New();
    channelConverter->setChannelsToRemove(false, false, false, true);
    channelConverter->setReverseChannels(true);
    channelConverter->setInputData(image);

    return channelConverter->updateAndGetOutputData<Image>();
}

std::shared_ptr<Image> ImagePyramidAccess::getPatchAsImage(int level, int offsetX, int offsetY, int width, int height) {
    if(width > 16384 || height > 16384)
        throw Exception("Image level is too large to convert into a FAST image");

    if(offsetX < 0 || offsetY < 0 || width <= 0 || height <= 0)
        throw Exception("Offset and size must be positive");

    if(offsetX + width >= m_image->getLevelWidth(level) || offsetY + height >= m_image->getLevelHeight(level))
        throw Exception("offset + size exceeds level size");

    auto image = Image::New();
    auto data = getPatchData(level, offsetX, offsetY, width, height);
    float scale = (float)m_image->getFullWidth()/m_image->getLevelWidth(level);
    image->create(width, height, TYPE_UINT8, m_image->getNrOfChannels(), std::move(data));
    image->setSpacing(Vector3f(
            scale,
            scale,
            1.0f
    ));
    // TODO Set transformation
    SceneGraph::setParentNode(image, std::dynamic_pointer_cast<SpatialDataObject>(m_image));

    if(m_fileHandle) {
        // Data is stored as BGRA, need to delete alpha channel and reverse it
        auto channelConverter = ImageChannelConverter::New();
        channelConverter->setChannelsToRemove(false, false, false, true);
        channelConverter->setReverseChannels(true);
        channelConverter->setInputData(image);
        return channelConverter->updateAndGetOutputData<Image>();
    }
    return image;
}

std::shared_ptr<Image> ImagePyramidAccess::getPatchAsImage(int level, int tileX, int tileY) {
    int levelWidth = m_image->getLevelWidth(level);
    int levelHeight = m_image->getLevelHeight(level);
    int tilesX = m_image->getLevelTilesX(level);
    int tilesY = m_image->getLevelTilesY(level);
    int levelTileWidth = m_image->getLevelTileWidth(level);
    int levelTileHeight = m_image->getLevelTileHeight(level);
    ImagePyramidPatch tile;
    tile.offsetX = tileX * levelTileWidth;
    tile.offsetY = tileY * levelTileHeight;

    tile.width = levelTileWidth;
    if(tileX == tilesX - 1)
        tile.width = levelWidth - tile.offsetX;
    tile.height = levelTileHeight;
    if(tileY == tilesY - 1)
        tile.height = levelHeight - tile.offsetY;

    // Read the actual data
    auto data = getPatchData(level, tile.offsetX, tile.offsetY, tile.width, tile.height);

    auto image = Image::New();
    float scale = (float)m_image->getFullWidth()/m_image->getLevelWidth(level);
    image->create(tile.width, tile.height, TYPE_UINT8, m_image->getNrOfChannels(), std::move(data));
    image->setSpacing(Vector3f(
            scale,
            scale,
            1.0f
    ));
    // TODO Set transformation
    SceneGraph::setParentNode(image, std::dynamic_pointer_cast<SpatialDataObject>(m_image));

    if(m_fileHandle != nullptr) {
        // Data is stored as BGRA, need to delete alpha channel and reverse it
        auto channelConverter = ImageChannelConverter::New();
        channelConverter->setChannelsToRemove(false, false, false, true);
        channelConverter->setReverseChannels(true);
        channelConverter->setInputData(image);

        return channelConverter->updateAndGetOutputData<Image>();
    } else {
        return image;
    }
    /*
    auto t = Image::New();
    t->create(512, 512, TYPE_UINT8, 1);
    t->fill(0);
    return t;
    */
}



}