#include "ImagePyramidAccess.hpp"
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Algorithms/ImageChannelConverter/ImageChannelConverter.hpp>
#include <FAST/Utility.hpp>
#if defined(__APPLE__) || defined(__MACOSX)
#include <openslide.h>
#else
#include <openslide/openslide.h>
#endif
#include <tiffio.h>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <FAST/Algorithms/NeuralNetwork/TensorToImage.hpp>
#include <FAST/Algorithms/ImageCaster/ImageCaster.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include <FAST/Algorithms/Compression/JPEGXLCompression.hpp>
#include <FAST/Algorithms/Compression/JPEGCompression.hpp>

namespace fast {

ImagePyramidAccess::ImagePyramidAccess(
        std::vector<ImagePyramidLevel> levels,
        openslide_t* fileHandle,
        TIFF* tiffHandle,
        std::shared_ptr<ImagePyramid> imagePyramid,
        bool write,
        std::unordered_set<std::string>& initializedPatchList,
        std::mutex& readMutex,
        ImageCompression compressionFormat
        ) : m_initializedPatchList(initializedPatchList), m_readMutex(readMutex) {
	if(levels.size() == 0)
		throw Exception("Image pyramid has no levels");
	m_image = imagePyramid;
	m_levels = levels;
	m_write = write;
    m_fileHandle = fileHandle;
    m_tiffHandle = tiffHandle;
    m_compressionFormat = compressionFormat;
}

void ImagePyramidAccess::release() {
	m_image->accessFinished();
}

ImagePyramidAccess::~ImagePyramidAccess() {
	release();
}

std::unique_ptr<uchar[]> ImagePyramidAccess::getPatchDataChar(int level, int x, int y, int width, int height) {
    const int levelWidth = m_image->getLevelWidth(level);
    const int levelHeight = m_image->getLevelHeight(level);
    const int channels = m_image->getNrOfChannels();
    const int tileWidth = m_image->getLevelTileWidth(level);
    const int tileHeight = m_image->getLevelTileHeight(level);
    const int bytesPerPixel = getSizeOfDataType(m_image->getDataType(), channels);
    // Store as uchar even though it might be something else
    auto data = std::make_unique<uchar[]>(width*height*bytesPerPixel);
    if(m_tiffHandle != nullptr) {
        // Read all tiles within region, then crop
        if(!isPatchInitialized(level, x, y)) {
            // Tile has not be initialized, fill with zeros and return..
            // TODO Do not try render these patches..
            std::memset(data.get(), channels > 1 ? 255 : 0, width*height*channels);
            return data;
        }
        std::lock_guard<std::mutex> lock(m_readMutex);
        if(m_image->isOMETIFF()) {
            if(TIFFCurrentDirOffset(m_tiffHandle) != m_levels[level].offset) {
                if(level == 0) {
                    TIFFSetDirectory(m_tiffHandle, level);
                    TIFFSetSubDirectory(m_tiffHandle, 0);
                } else {
                    TIFFSetSubDirectory(m_tiffHandle, m_levels[level].offset);
                }
            }
        } else {
            if(TIFFCurrentDirectory(m_tiffHandle) != level)
                TIFFSetDirectory(m_tiffHandle, level);
        }
        if(width == tileWidth && height == tileHeight && x % tileWidth == 0 && y % tileHeight == 0) {
            // From TIFFReadTile documentation: Return the data for the tile containing the specified coordinates.
            int bytesRead = readTileFromTIFF((void *) data.get(), x, y, level);
        } else if(width <= tileWidth && height <= tileHeight && x % tileWidth == 0 && y % tileHeight == 0) {
            auto tileData = std::make_unique<uchar[]>(tileWidth*tileHeight*bytesPerPixel);
            {
                // From TIFFReadTile documentation: Return the data for the tile containing the specified coordinates.
                // In TIFF all tiles have the same size, thus they are padded..
                int bytesRead = readTileFromTIFF((void *) tileData.get(), x, y, level);
            }
            // Remove extra
            for(int dy = 0; dy < height; ++dy) {
                for(int dx = 0; dx < width; ++dx) {
                    for(int channel = 0; channel < channels; ++channel) {
                        data[(dx + dy*width)*channels + channel] = tileData[(dx + dy*tileWidth)*channels + channel];
                    }
                }
            }
        } else {
            // Create buffer to contain all tiles
            int totalTilesX = std::ceil((float)width / tileWidth);
            int totalTilesY = std::ceil((float)height / tileHeight);
            if(x % tileWidth != 0) totalTilesX += 1;
            if(y % tileHeight != 0) totalTilesY += 1;
            const int targetNumberOfTiles = totalTilesX*totalTilesY;
            auto fullTileBuffer = make_uninitialized_unique<uchar[]>(tileWidth*tileHeight*targetNumberOfTiles*bytesPerPixel);
            // Does the buffer need to be initialized/padded?
            if(x+width >= levelWidth || x+height >= levelHeight) { // Some tiles are outside, fill it
                if(channels > 1) {
                    std::memset(fullTileBuffer.get(), 255, tileWidth*tileHeight*targetNumberOfTiles*channels);
                } else {
                    std::memset(fullTileBuffer.get(), 0, tileWidth*tileHeight*targetNumberOfTiles*channels);
                }
            }
            // Fill the full tile buffer
            const int firstTileX = x / tileWidth;
            const int firstTileY = y / tileHeight;
            const int fullTileBufferWidth = totalTilesX*tileWidth;
            for(int i = 0; i < totalTilesX; ++i) {
                for(int j = 0; j < totalTilesY; ++j) {
                    auto tileData = make_uninitialized_unique<uchar[]>(tileWidth*tileHeight*bytesPerPixel);
                    int tileX = i*tileWidth;
                    int tileY = j*tileHeight;
                    int bytesRead = readTileFromTIFF((void *) tileData.get(), firstTileX*tileWidth+tileX, firstTileY*tileHeight+tileY, level);
                    // Stitch tile into full buffer
                    for(int cy = 0; cy < tileHeight; ++cy) {
                        for(int cx = 0; cx < tileWidth; ++cx) {
                            for(int byte = 0; byte < bytesPerPixel; ++byte) {
                                fullTileBuffer[(tileX + cx + (tileY + cy)*fullTileBufferWidth)*bytesPerPixel + byte] = tileData[(cx + cy*tileWidth)*bytesPerPixel + byte];
                            }
                        }
                    }
                }
            }
            // Crop the full buffer to data[]
            const int offsetX = x - firstTileX*tileWidth;
            const int offsetY = y - firstTileY*tileHeight;
            for(int cy = offsetY; cy < offsetY + height; ++cy) {
                for(int cx = offsetX; cx < offsetX + width; ++cx) {
                    for(int byte = 0; byte < bytesPerPixel; ++byte) {
                        data[(cx - offsetX + (cy - offsetY) * width)*bytesPerPixel + byte] = fullTileBuffer[(cx + cy * fullTileBufferWidth)*bytesPerPixel + byte];
                    }
                }
            }
        }
    } else if(m_fileHandle != nullptr) {
        int scale = (float)m_image->getFullWidth()/levelWidth;
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

std::shared_ptr<Image> ImagePyramidAccess::getLevelAsImage(int level) {
    if(level < 0 || level >= m_image->getNrOfLevels())
        throw Exception("Incorrect level given to getLevelAsImage" + std::to_string(level));

    int width = m_image->getLevelWidth(level);
    int height = m_image->getLevelHeight(level);
    if(width > 16384 || height > 16384)
        throw Exception("Image level is too large to convert into a FAST image");

    return getPatchAsImage(level, 0, 0, width, height, true);
}

std::shared_ptr<Image> ImagePyramidAccess::getPatchAsImage(int level, int offsetX, int offsetY, int width, int height, bool convertToRGB) {
    if(width > 16384 || height > 16384)
        throw Exception("Image level is too large to convert into a FAST image");

    if(offsetX < 0 || offsetY < 0 || width <= 0 || height <= 0)
        throw Exception("Offset and size must be positive");

    if(offsetX + width > m_image->getLevelWidth(level) || offsetY + height > m_image->getLevelHeight(level))
        throw Exception("offset + size exceeds level size");

    Image::pointer image;
    switch(m_image->getDataType()) {
        case TYPE_UINT8:
            {
                auto data = getPatchData<uchar>(level, offsetX, offsetY, width, height);
                image = Image::create(width, height, m_image->getDataType(), m_image->getNrOfChannels(), std::move(data));
            }
            break;
        case TYPE_UINT16:
            {
                auto data = getPatchData<ushort>(level, offsetX, offsetY, width, height);
                image = Image::create(width, height, m_image->getDataType(), m_image->getNrOfChannels(), std::move(data));
            }
            break;
        case TYPE_INT8:
            {
                auto data = getPatchData<char>(level, offsetX, offsetY, width, height);
                image = Image::create(width, height, m_image->getDataType(), m_image->getNrOfChannels(), std::move(data));
            }
            break;
        case TYPE_INT16:
            {
                auto data = getPatchData<short>(level, offsetX, offsetY, width, height);
                image = Image::create(width, height, m_image->getDataType(), m_image->getNrOfChannels(), std::move(data));
            }
            break;
        case TYPE_FLOAT:
            {
                auto data = getPatchData<float>(level, offsetX, offsetY, width, height);
                image = Image::create(width, height, m_image->getDataType(), m_image->getNrOfChannels(), std::move(data));
            }
            break;
    }
    float scale = m_image->getLevelScale(level);
    auto spacing = m_image->getSpacing();
    image->setSpacing(Vector3f(
            spacing.x()*scale,
            spacing.y()*scale,
            1.0f
    ));
    // Set transformation
    auto T = Transform::create(Vector3f(offsetX*scale, offsetY*scale, 0.0f));
    image->setTransform(T);
    SceneGraph::setParentNode(image, std::dynamic_pointer_cast<SpatialDataObject>(m_image));

    if(m_fileHandle != nullptr && convertToRGB) {
        // Data is stored as BGRA, need to delete alpha channel and reverse it
        auto channelConverter = ImageChannelConverter::New();
        channelConverter->setChannelsToRemove(false, false, false, true);
        channelConverter->setReverseChannels(true);
        channelConverter->setInputData(image);
        image = channelConverter->updateAndGetOutputData<Image>();
    }
    return image;
}

std::shared_ptr<Image> ImagePyramidAccess::getPatchAsImageForMagnification(float magnification, float offsetX, float offsetY, int width, int height, bool convertToRGB) {
    if(width > 16384 || height > 16384)
        throw Exception("Image level is too large to convert into a FAST image");

    if(offsetX < 0 || offsetY < 0 || width <= 0 || height <= 0)
        throw Exception("Offset and size must be positive");

    int level;
    float resampleFactor = 1.0f;
    std::tie(level, resampleFactor) = m_image->getClosestLevelForMagnification(magnification);

    // Convert offset from physical float position to pixel position in level
    offsetX = round((offsetX / m_image->getSpacing().x())/m_image->getLevelScale(level));
    offsetY = round((offsetY / m_image->getSpacing().y())/m_image->getLevelScale(level));

    const int patchWidth = width*resampleFactor;
    const int patchHeight = height*resampleFactor;

    auto image = getPatchAsImage(level, offsetX, offsetY, patchWidth, patchHeight, convertToRGB);

    if(resampleFactor > 1.0f) {
        image = ImageResizer::create(width, height, 1, m_image->getNrOfChannels() > 1)->connect(image)->runAndGetOutputData<Image>();
    }

    return image;
}

std::shared_ptr<Image> ImagePyramidAccess::getPatchAsImage(int level, int tileX, int tileY, bool convertToRGB) {
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

    return getPatchAsImage(level, tile.offsetX, tile.offsetY, tile.width, tile.height, convertToRGB);
}

uint32_t ImagePyramidAccess::writeTileToTIFF(int level, int x, int y, uchar *data, int width, int height, int channels) {
    if(m_image->getCompression() == ImageCompression::NEURAL_NETWORK) {
        auto image = Image::create(width, height, TYPE_UINT8, channels, data); // TODO this seems unnecessary
        return writeTileToTIFF(level, x, y, image);
    } else if(m_image->getCompression() == ImageCompression::JPEG) {
        return writeTileToTIFFJPEG(level, x, y, data);
    } else if(m_image->getCompression() == ImageCompression::JPEGXL) {
        return writeTileToTIFFJPEGXL(level, x, y, data);
    } else {
        return writeTileToTIFF(level, x, y, data);
    }
}

uint32_t ImagePyramidAccess::writeTileToTIFF(int level, int x, int y, Image::pointer image) {
    if(m_image->getCompression() == ImageCompression::NEURAL_NETWORK) {
        return writeTileToTIFFNeuralNetwork(level, x, y, image);
    } else if(m_image->getCompression() == ImageCompression::JPEGXL) {
        auto access = image->getImageAccess(ACCESS_READ);
        return writeTileToTIFFJPEGXL(level, x, y, (uchar*)access->get());
    } else if(m_image->getCompression() == ImageCompression::JPEG) {
        auto access = image->getImageAccess(ACCESS_READ);
        return writeTileToTIFFJPEG(level, x, y, (uchar*)access->get());
    } else {
        auto access = image->getImageAccess(ACCESS_READ);
        return writeTileToTIFF(level, x, y, (uchar*)access->get());
    }
}

uint32_t ImagePyramidAccess::writeTileToTIFF(int level, int x, int y, uchar *data) {
    std::lock_guard<std::mutex> lock(m_readMutex);
    TIFFSetDirectory(m_tiffHandle, level);
    TIFFWriteTile(m_tiffHandle, (void *) data, x, y, 0, 0);
    TIFFCheckpointDirectory(m_tiffHandle);
    uint32_t tile_id = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    return tile_id;
}

uint32_t ImagePyramidAccess::writeTileToTIFFJPEGXL(int level, int x, int y, uchar *data) {
    std::lock_guard<std::mutex> lock(m_readMutex);
    TIFFSetDirectory(m_tiffHandle, level);
    uint32_t tile_id = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    JPEGXLCompression jxl;
    std::vector<uchar> compressed;
    jxl.compress(data, m_image->getLevelTileWidth(level), m_image->getLevelTileHeight(level), &compressed, m_image->getCompressionQuality());
    TIFFSetWriteOffset(m_tiffHandle, 0); // Set write offset to 0, so that we dont appen data
    TIFFWriteRawTile(m_tiffHandle, tile_id, (void *) compressed.data(), compressed.size()); // This appends data..
    TIFFCheckpointDirectory(m_tiffHandle);
    return tile_id;
}


uint32_t ImagePyramidAccess::writeTileToTIFFJPEG(int level, int x, int y, uchar *data) {
    std::lock_guard<std::mutex> lock(m_readMutex);
    TIFFSetDirectory(m_tiffHandle, level);
    uint32_t tile_id = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    JPEGCompression jpeg;
    std::vector<uchar> compressed;
    jpeg.compress(data, m_image->getLevelTileWidth(level), m_image->getLevelTileHeight(level), &compressed, m_image->getCompressionQuality());
    TIFFSetWriteOffset(m_tiffHandle, 0); // Set write offset to 0, so that we dont appen data
    TIFFWriteRawTile(m_tiffHandle, tile_id, (void *) compressed.data(), compressed.size()); // This appends data..
    TIFFCheckpointDirectory(m_tiffHandle);
    return tile_id;
}

uint32_t ImagePyramidAccess::writeTileToTIFFNeuralNetwork(int level, int x, int y, Image::pointer image) {
    std::lock_guard<std::mutex> lock(m_readMutex);
    TIFFSetDirectory(m_tiffHandle, level);
    uint32_t tile_id = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    if(m_image->getCompression() != ImageCompression::NEURAL_NETWORK)
        throw Exception("Compression is not neural network type");

    auto compressionModel = m_image->getCompressionModel();
    compressionModel->connect(image);
    auto tensor = compressionModel->runAndGetOutputData<Tensor>();
    auto access = tensor->getAccess(ACCESS_READ);
    float* data = access->getRawData();
    uint32_t size = tensor->getShape().getTotalSize()*4;
    TIFFSetWriteOffset(m_tiffHandle, 0); // Set write offset to 0, so that we dont appen data
    TIFFWriteRawTile(m_tiffHandle, tile_id, (void *) data, size); // This appends data..
    //TIFFWriteTile(m_tiffHandle, (void *) data, x, y, 0,0); // This does not append, but tries to compress data

    TIFFCheckpointDirectory(m_tiffHandle);
    return tile_id;
}

void ImagePyramidAccess::setPatch(int level, int x, int y, Image::pointer patch, bool propagate) {
    if(m_tiffHandle == nullptr)
        throw Exception("setPatch only available for TIFF backend ImagePyramids");

    if(m_image->getLevelTileWidth(level) > patch->getWidth() || m_image->getLevelTileHeight(level) > patch->getHeight()) {
        // Padding needed
        auto paddedImage = Image::create(m_image->getLevelTileWidth(level), m_image->getLevelTileHeight(level), patch->getDataType(), m_image->getNrOfChannels());
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

    // Write tile to this level
    uint32_t tile_id = writeTileToTIFF(level, x, y, patch);
    m_initializedPatchList.insert(std::to_string(level) + "-" + std::to_string(tile_id));

    // Add patch to list of dirty patches, so the renderer can update it if needed
    int levelWidth = m_image->getLevelWidth(level);
    int levelHeight = m_image->getLevelHeight(level);
    const int tilesX = m_image->getLevelTilesX(level);
    const int tilesY = m_image->getLevelTilesY(level);
    int patchIdX = std::floor(((float)x / levelWidth) * tilesX);
    int patchIdY = std::floor(((float)y / levelHeight) * tilesY);
    m_image->setDirtyPatch(level, patchIdX, patchIdY);

    // Propagate upwards
    if(propagate)
        propagatePatch(patch, level, x, y);
}

void ImagePyramidAccess::propagatePatch(Image::pointer patch, int level, int x, int y) {
    auto previousData = std::make_unique<uchar[]>(patch->getNrOfVoxels()*patch->getNrOfChannels());
    auto patchAccess = patch->getImageAccess(ACCESS_READ);
    auto data = (uchar*)patchAccess->get();
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
        auto newData = getPatchData<uchar>(level, x, y, tileWidth, tileHeight);

        // Downsample tile from previous level and add it to existing tile
        if(m_image->getNrOfChannels() >= 3) {
            const int channels = m_image->getNrOfChannels();
            // Use average if RGB(A) image
            for(int dy = 0; dy < tileHeight/2; ++dy) {
                for(int dx = 0; dx < tileWidth/2; ++dx) {
                    for(int c = 0; c < channels; ++c) {
                        newData[c + channels*(dx + offsetX * tileWidth / 2 + (dy + offsetY * tileHeight / 2) * tileWidth)] =
                                (uchar)round((float)(
                                        previousData[c + channels*(dx * 2 + dy * 2 * previousTileWidth)] +
                                        previousData[c + channels*(dx * 2 + 1 + dy * 2 * previousTileWidth)] +
                                        previousData[c + channels*(dx * 2 + 1 + (dy * 2 + 1) * previousTileWidth)] +
                                        previousData[c + channels*(dx * 2 + (dy * 2 + 1) * previousTileWidth)]
                                        ) / 4);
                    }
                }
            }
        } else {
            // Use majority vote if single channel image.
            for(int dy = 0; dy < tileHeight/2; ++dy) {
                for(int dx = 0; dx < tileWidth/2; ++dx) {
                    /*
                    // This is more correct, but 100 times slower than just doing max.
                    std::vector<uchar> list = {
                            previousData[dx*2 + dy*2*previousTileWidth],
                            previousData[dx*2 + 1 + dy*2*previousTileWidth],
                            previousData[dx*2 + 1 + (dy*2+1)*previousTileWidth],
                            previousData[dx*2 + (dy*2+1)*previousTileWidth]
                    };
                    std::sort(list.begin(), list.end());
                    if(list[0] == list[1]) { // If there is more than of element 0, it should be placed as element 1
                        newData[dx + offsetX*tileWidth/2 + (dy+offsetY*tileHeight/2)*tileWidth] = list[0];
                    } else { // If not, it means that there is more than 1 of element 1, OR all 4 values are different and its no matter which is picked.
                        newData[dx + offsetX*tileWidth/2 + (dy+offsetY*tileHeight/2)*tileWidth] = list[2];
                    }*/

                    // Just do max? 0.006 milliseconds
                    uchar list[4] = {
                            previousData[dx*2 + dy*2*previousTileWidth],
                            previousData[dx*2 + 1 + dy*2*previousTileWidth],
                            previousData[dx*2 + 1 + (dy*2+1)*previousTileWidth],
                            previousData[dx*2 + (dy*2+1)*previousTileWidth]
                    };
                    newData[dx + offsetX*tileWidth/2 + (dy+offsetY*tileHeight/2)*tileWidth] = std::max(std::max(std::max(list[0], list[1]), list[2]), list[3]);
                }
            }
        }
        auto tile_id = writeTileToTIFF(level, x, y, newData.get(), tileWidth, tileHeight, channels);
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

bool ImagePyramidAccess::isPatchInitialized(int level, int x, int y) {
    if(m_image->isPyramidFullyInitialized())
        return true;
    std::lock_guard<std::mutex> lock(m_readMutex);
    TIFFSetDirectory(m_tiffHandle, level);
    auto tile = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    return m_initializedPatchList.count(std::to_string(level) + "-" + std::to_string(tile)) > 0;
}

int ImagePyramidAccess::readTileFromTIFF(void *data, int x, int y, int level) {
    const auto tileWidth = m_image->getLevelTileWidth(level);
    const auto tileHeight = m_image->getLevelTileHeight(level);
    const auto channels = m_image->getNrOfChannels();
    TIFFSetDirectory(m_tiffHandle, level);
    const uint32_t tile_id = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    if(TIFFGetStrileByteCount(m_tiffHandle, tile_id) == 0) { // Blank patch
        if(channels == 1) {
            std::memset(data, 0, tileWidth*tileHeight*channels);
        } else {
            std::memset(data, 255, tileWidth*tileHeight*channels);
        }
        return 0;
    }
    // Assumes level (directory is already set)
    if(m_compressionFormat == ImageCompression::NEURAL_NETWORK) {
        auto decompressionModel = m_image->getDecompressionModel();
        // TODO The logic here must be improved
        // TODO this assumes fixed size code
        auto shape = decompressionModel->getInputNodes().begin()->second.shape;
        shape[0] = 1;
        int64_t size = shape.getTotalSize()*4;
        float* buffer = new float[shape.getTotalSize()];
        int bytesRead = TIFFReadRawTile(m_tiffHandle, tile_id, buffer, size);
        auto tensor = Tensor::create(buffer, shape);
        decompressionModel->connect(tensor);
        // TODO TensorToImage not really needed..
        auto outputTensor = decompressionModel->runAndGetOutputData<Tensor>();
        auto tensorToImage = TensorToImage::create()->connect(outputTensor);
        auto image = tensorToImage->runAndGetOutputData<Image>();
        // Have to go from float image to uint8 image
        image = ImageCaster::create(TYPE_UINT8, m_image->getDecompressionOutputScaleFactor())->connect(image)->runAndGetOutputData<Image>();
        auto access = image->getImageAccess(ACCESS_READ);
        std::memcpy(data, access->get(), image->getNrOfVoxels()*image->getNrOfChannels());
        return bytesRead;
    } else {
        int bytesRead = 0;
        if(m_compressionFormat == ImageCompression::JPEG /*&& m_image->isOMETIFF()*/) {
            // Use libjpeg for decompression, as ome-tiff files doesn't seem to like tiff's internal jpeg
            auto buffer = make_uninitialized_unique<char[]>(tileWidth*tileHeight*channels);
            bytesRead = TIFFReadRawTile(m_tiffHandle, tile_id, buffer.get(), tileWidth*tileHeight*channels);

            JPEGCompression jpeg(m_JPEGTablesCount, m_JPEGTablesData);
            int width, height;
            jpeg.decompress((uchar*)buffer.get(), bytesRead, &width, &height, (uchar*)data);
        } else if(m_compressionFormat == ImageCompression::JPEGXL) {
            auto buffer = make_uninitialized_unique<char[]>(tileWidth*tileHeight*channels);
            bytesRead = TIFFReadRawTile(m_tiffHandle, tile_id, buffer.get(), tileWidth*tileHeight*channels);

            JPEGXLCompression jxl;
            int width, height;
            jxl.decompress((uchar*)buffer.get(), bytesRead, &width, &height, (uchar*)data);
        } else {
            bytesRead = TIFFReadTile(m_tiffHandle, data, x, y, 0, 0);
        }
        return bytesRead;
    }
}

void ImagePyramidAccess::setBlankPatch(int level, int x, int y) {
    if(m_tiffHandle == nullptr)
        throw Exception("setBlankPatch only available for TIFF backend ImagePyramids");

    std::lock_guard<std::mutex> lock(m_readMutex);
    TIFFSetDirectory(m_tiffHandle, level);
    uint32_t tile_id = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    TIFFSetWriteOffset(m_tiffHandle, 0); // Set write offset to 0, so that we dont appen data
    // Writing zero bytes, will produce a warning, thus we write 1 byte
    char data = 0;
    TIFFWriteRawTile(m_tiffHandle, tile_id, &data, 0);
    TIFFCheckpointDirectory(m_tiffHandle);
    m_initializedPatchList.insert(std::to_string(level) + "-" + std::to_string(tile_id));

    // TODO Propagate or not?
}

void ImagePyramidAccess::setJPEGTables(uint32_t tableCount, void *tableData) {
    m_JPEGTablesCount = tableCount;
    m_JPEGTablesData = tableData;
}


}
