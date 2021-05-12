#include "ImagePyramidAccess.hpp"
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Algorithms/ImageChannelConverter/ImageChannelConverter.hpp>
#include <FAST/Utility.hpp>
#include <openslide/openslide.h>
#include <tiffio.h>
#include <FAST/Data/Image.hpp>

namespace fast {

ImagePyramidAccess::ImagePyramidAccess(std::vector<ImagePyramidLevel> levels, openslide_t* fileHandle, TIFF* tiffHandle, std::shared_ptr<ImagePyramid> imagePyramid, bool write, std::unordered_set<std::string>& initializedPatchList) : m_initializedPatchList(initializedPatchList) {
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
        if(!isPatchInitialized(level, x, y)) {
            // Tile has not be initialized, fill with zeros and return..
            // TODO Do not try render these patches..
            std::memset(data.get(), 0, width*height*channels);
            return data;
        }
        if(width == tileWidth && height == tileHeight) {
            std::lock_guard<std::mutex> lock(m_tiffMutex);
            TIFFSetDirectory(m_tiffHandle, level);
            int bytesRead = TIFFReadTile(m_tiffHandle, (void *) data.get(), x, y, 0, 0);
        } else if(width < tileWidth || height < tileHeight) {
            auto tileData = std::make_unique<uchar[]>(tileWidth*tileHeight*channels);
            {
                std::lock_guard<std::mutex> lock(m_tiffMutex);
                TIFFSetDirectory(m_tiffHandle, level);
                // In TIFF all tiles have the same size, thus they are padded..
                int bytesRead = TIFFReadTile(m_tiffHandle, (void *) tileData.get(), x, y, 0, 0);
            }
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
    auto spacing = m_image->getSpacing();
    image->create(width, height, TYPE_UINT8, m_image->getNrOfChannels(), std::move(data));
    // TODO, we should have added spacing here to scale, but then rendering doesn't work
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
}

void ImagePyramidAccess::setPatch(int level, int x, int y, Image::pointer patch) {
    if(m_tiffHandle == nullptr)
        throw Exception("setPatch only available for TIFF backend ImagePyramids");

    if(m_image->getLevelTileWidth(level) > patch->getWidth() || m_image->getLevelTileHeight(level) > patch->getHeight()) {
        // Padding needed
        auto paddedImage = Image::New();
        paddedImage->create(m_image->getLevelTileWidth(level), m_image->getLevelTileHeight(level), patch->getDataType(), m_image->getNrOfChannels());
        if(m_image->getNrOfChannels() >= 3) {
            paddedImage->fill(255);
        } else {
            paddedImage->fill(0);
        }
        auto device = std::dynamic_pointer_cast<OpenCLDevice>(DeviceManager::getInstance()->getDefaultDevice());
        {
            auto dest = paddedImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            auto src = patch->getOpenCLImageAccess(ACCESS_READ, device);
            device->getCommandQueue().enqueueCopyImage(*src->get2DImage(), *dest->get2DImage(),
                                                       createOrigoRegion(), createOrigoRegion(),
                                                       createRegion(patch->getSize()));
            device->getCommandQueue().finish();
        }
        patch = paddedImage;
    }
    auto patchAccess = patch->getImageAccess(ACCESS_READ);
    auto data = (uchar*)patchAccess->get();
    uint32_t tile_id;
    {
        std::lock_guard<std::mutex> lock(m_tiffMutex);
        TIFFSetDirectory(m_tiffHandle, level);
        TIFFWriteTile(m_tiffHandle, (void *) data, x, y, 0, 0);
        TIFFCheckpointDirectory(m_tiffHandle);
        tile_id = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    }
    m_initializedPatchList.insert(std::to_string(level) + "-" + std::to_string(tile_id));

    // Add patch to list of dirty patches, so the renderer can update it if needed
    int levelWidth = m_image->getLevelWidth(level);
    int levelHeight = m_image->getLevelHeight(level);
    const int tilesX = m_image->getLevelTilesX(level);
    const int tilesY = m_image->getLevelTilesY(level);
    int patchIdX = std::floor(((float)x / levelWidth) * tilesX);
    int patchIdY = std::floor(((float)y / levelHeight) * tilesY);
    m_image->setDirtyPatch(level, patchIdX, patchIdY);

    // TODO Propagate upwards
    auto previousData = std::make_unique<uchar[]>(patch->getNrOfVoxels()*patch->getNrOfChannels());
    std::memcpy(previousData.get(), data, patch->getNrOfVoxels()*patch->getNrOfChannels());
    const auto channels = m_image->getNrOfChannels();
    while(level < m_image->getNrOfLevels()-1) {
        const auto previousTileWidth = m_image->getLevelTileWidth(level);
        const auto previousTileHeight = m_image->getLevelTileHeight(level);
        ++level;
        x /= 2;
        y /= 2;
        const auto tileWidth = m_image->getLevelTileWidth(level);
        const auto tileHeight = m_image->getLevelTileHeight(level);
        int offsetX = x % tileWidth > 0 ? 1 : 0;
        int offsetY = y % tileHeight > 0 ? 1 : 0;
        x -= offsetX*tileWidth/2;
        y -= offsetY*tileHeight/2;

        // Get existing tile. This gets the tile in which x, y is contained in, not where it starts..
        auto newData = getPatchData(level, x, y, tileWidth, tileHeight);

        // Downsample tile from previous level and add it to existing tile
        for(int dy = 0; dy < tileHeight/2; ++dy) {
            for(int dx = 0; dx < tileWidth/2; ++dx) {
                newData[dx + offsetX*tileWidth/2 + (dy+offsetY*tileHeight/2)*tileWidth] =
                        (uchar)round((float)(previousData[dx*2 + dy*2*previousTileWidth] +
                        previousData[dx*2 + 1 + dy*2*previousTileWidth] +
                        previousData[dx*2 + 1 + (dy*2+1)*previousTileWidth] +
                        previousData[dx*2 + (dy*2+1)*previousTileWidth])/4);
            }
        }
        {
            std::lock_guard<std::mutex> lock(m_tiffMutex);
            TIFFSetDirectory(m_tiffHandle, level);
            TIFFWriteTile(m_tiffHandle, (void *) newData.get(), x, y, 0, 0);
            TIFFCheckpointDirectory(m_tiffHandle);
            tile_id = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
        }
        previousData = std::move(newData);

        int levelWidth = m_image->getLevelWidth(level);
        int levelHeight = m_image->getLevelHeight(level);
        const int tilesX = m_image->getLevelTilesX(level);
        const int tilesY = m_image->getLevelTilesY(level);
        int patchIdX = std::floor(((float)x / levelWidth) * tilesX);
        int patchIdY = std::floor(((float)y / levelHeight) * tilesY);
        m_image->setDirtyPatch(level, patchIdX, patchIdY);
        m_initializedPatchList.insert(std::to_string(level) + "-" + std::to_string(tile_id));
    }
}

bool ImagePyramidAccess::isPatchInitialized(uint level, uint x, uint y) {
    if(m_image->isPyramidFullyInitialized())
        return true;
    std::lock_guard<std::mutex> lock(m_tiffMutex);
    TIFFSetDirectory(m_tiffHandle, level);
    auto tile = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    return m_initializedPatchList.count(std::to_string(level) + "-" + std::to_string(tile)) > 0;
}

}