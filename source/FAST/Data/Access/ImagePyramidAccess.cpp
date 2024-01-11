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
#include <jpeglib.h>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <FAST/Algorithms/NeuralNetwork/TensorToImage.hpp>
#include <FAST/Algorithms/ImageCaster/ImageCaster.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>

namespace fast {

ImagePyramidAccess::ImagePyramidAccess(
        std::vector<ImagePyramidLevel> levels,
        openslide_t* fileHandle,
        TIFF* tiffHandle,
        std::ifstream* vsiHandle,
        std::vector<vsi_tile_header>& vsiTiles,
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
    m_vsiHandle = vsiHandle;
    m_vsiTiles = vsiTiles;
    m_compressionFormat = compressionFormat;
}

void ImagePyramidAccess::release() {
	m_image->accessFinished();
}

ImagePyramidAccess::~ImagePyramidAccess() {
	release();
}

void jpegErrorExit(j_common_ptr cinfo) {
    char jpegLastErrorMsg[JMSG_LENGTH_MAX];
    // Create message
    ( *( cinfo->err->format_message ) ) ( cinfo, jpegLastErrorMsg );
    throw std::runtime_error( jpegLastErrorMsg );
}

void ImagePyramidAccess::readVSITileToBuffer(vsi_tile_header tile, uchar* data) {
    if(m_compressionFormat == ImageCompression::JPEG) {
        auto buffer = make_uninitialized_unique<char[]>(tile.numbytes);

        // Reading VSI tiles is not thread safe
        {
            std::lock_guard<std::mutex> lock(m_readMutex);
            m_vsiHandle->seekg(tile.offset);
            m_vsiHandle->read(buffer.get(), tile.numbytes);
        }
        jpeg_decompress_struct cinfo;
        jpeg_error_mgr jerr; //error handling
        jpeg_source_mgr src_mem;
        jerr.error_exit = jpegErrorExit;
        cinfo.err = jpeg_std_error(&jerr);
        try {
            jpeg_create_decompress(&cinfo);
            jpeg_mem_src(&cinfo, (uchar*)buffer.get(), tile.numbytes);
            int ret = jpeg_read_header(&cinfo, false);
            if(ret != 1) {
                throw Exception("Jpeg error..");
            }
            //cinfo.jpeg_color_space = JCS_YCbCr;
            //cinfo.jpeg_color_space = JCS_RGB;
            jpeg_start_decompress(&cinfo);
            unsigned char* line = data;
            while (cinfo.output_scanline < cinfo.output_height) {
                jpeg_read_scanlines (&cinfo, &line, 1);
                line += 3*cinfo.output_width;
            }
            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);
        } catch(std::exception &e) {
            jpeg_destroy_decompress( &cinfo );
            throw Exception("JPEG error: " + std::string(e.what())); // or return an error code
        }
    } else if(m_compressionFormat == ImageCompression::RAW) { // Uncompressed
        // Reading VSI tiles is not thread safe
        {
            auto buffer = make_uninitialized_unique<char[]>(tile.numbytes);
            std::lock_guard<std::mutex> lock(m_readMutex);
            m_vsiHandle->seekg(tile.offset);
            m_vsiHandle->read(buffer.get(), tile.numbytes);
            // Data is stored as BGR, convert it to RGB
            // TODO could optimize this by doing it on the GPU instead..
            for(int i = 0; i < tile.numbytes/3; ++i) {
                data[i*3 + 0] = buffer[i*3+2];
                data[i*3 + 1] = buffer[i*3+1];
                data[i*3 + 2] = buffer[i*3+0];
            }
        }
    } else {
        throw Exception("Unknown image compression format in ImagePyramidAccess::readVSITileToBuffer: " + std::to_string((int)m_compressionFormat));
    }
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
            std::memset(data.get(), 0, width*height*channels);
            return data;
        }
        std::lock_guard<std::mutex> lock(m_readMutex);
        if(m_image->isOMETIFF()) {
            if(level == 0) {
                TIFFSetDirectory(m_tiffHandle, level);
            } else {
                TIFFSetSubDirectory(m_tiffHandle, m_levels[level].offset);
            }
        } else {
            TIFFSetDirectory(m_tiffHandle, level);
        }
        if(width == tileWidth && height == tileHeight && x % tileWidth == 0 && y % tileHeight == 0) {
            // From TIFFReadTile documentation: Return the data for the tile containing the specified coordinates.
            int bytesRead = readTileFromTIFF((void *) data.get(), x, y);
        } else if((width < tileWidth || height < tileHeight) && x % tileWidth == 0 && y % tileHeight == 0) {
            auto tileData = std::make_unique<uchar[]>(tileWidth*tileHeight*bytesPerPixel);
            {
                // From TIFFReadTile documentation: Return the data for the tile containing the specified coordinates.
                // In TIFF all tiles have the same size, thus they are padded..
                int bytesRead = readTileFromTIFF((void *) tileData.get(), x, y);
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
                    auto tileData = std::make_unique<uchar[]>(tileWidth*tileHeight*bytesPerPixel);
                    int tileX = i*tileWidth;
                    int tileY = j*tileHeight;
                    int bytesRead = readTileFromTIFF((void *) tileData.get(), firstTileX*tileWidth+tileX, firstTileY*tileHeight+tileY);
                    // Stitch tile into full buffer
                    for(int cy = 0; cy < tileHeight; ++cy) {
                        for(int cx = 0; cx < tileWidth; ++cx) {
                            for(int channel = 0; channel < channels; ++channel) {
                                for(int byte = 0; byte < bytesPerPixel; ++byte) {
                                    fullTileBuffer[((tileX + cx + (tileY + cy)*fullTileBufferWidth)*channels + channel)*bytesPerPixel + byte] = tileData[((cx + cy*tileWidth)*channels + channel)*bytesPerPixel + byte];
                                }
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
                    for(int channel = 0; channel < channels; ++channel) {
                        for(int byte = 0; byte < bytesPerPixel; ++byte) {
                            data[((cx - offsetX + (cy - offsetY) * width)*channels + channel)*bytesPerPixel + byte] = fullTileBuffer[((cx + cy * fullTileBufferWidth)*channels + channel)*bytesPerPixel + byte];
                        }
                    }
                }
            }
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
    } else if(!m_vsiTiles.empty()) {
        if(width == tileWidth && height == tileHeight && x % tileWidth == 0 && y % tileHeight == 0) {
            vsi_tile_header tile;
            bool found = false;
            for(int i = 0; i < m_vsiTiles.size(); ++i) {
                vsi_tile_header currentTile = m_vsiTiles[i];
                if(currentTile.level != level)
                    continue;
                if(currentTile.coord[0] != x / m_image->getLevelTileWidth(level) || currentTile.coord[1] != y / m_image->getLevelTileHeight(level))
                    continue;
                tile = currentTile;
                found = true;
                break;
            }
            if(!found)
                throw Exception("Could not find tile for getPathcData in VSI");

            readVSITileToBuffer(tile, data.get());
        } else {
            std::vector<vsi_tile_header> tilesToRead;
            int firstTileX = x / m_image->getLevelTileWidth(level);
            int firstTileY = y / m_image->getLevelTileHeight(level);
            int lastTileX = std::ceil((float)(x + width) / m_image->getLevelTileWidth(level));
            int lastTileY = std::ceil((float)(y + height) / m_image->getLevelTileHeight(level));
            // How many tiles we are supposed to have
            const int targetNumberOfTiles = (lastTileX-firstTileX)*(lastTileY-firstTileY);
            for(int i = 0; i < m_vsiTiles.size(); ++i) {
                vsi_tile_header currentTile = m_vsiTiles[i];
                if(currentTile.level != level)
                    continue;
                if(
                        !((currentTile.coord[0] >= firstTileX && currentTile.coord[0] < lastTileX) &&
                        (currentTile.coord[1] >= firstTileY && currentTile.coord[1] < lastTileY)))
                    continue;
                tilesToRead.push_back(currentTile);
            }
            if(tilesToRead.empty())
                throw Exception("Tiles to ready empty");

            auto fullTileBuffer = make_uninitialized_unique<uchar[]>(tileWidth*tileHeight*targetNumberOfTiles*channels);
            if(tilesToRead.size() != targetNumberOfTiles) { // Some tiles are missing.. (edge case) fill with some blank value
                if(channels > 1) {
                    std::memset(fullTileBuffer.get(), 255, tileWidth*tileHeight*targetNumberOfTiles*channels);
                } else {
                    std::memset(fullTileBuffer.get(), 0, tileWidth*tileHeight*targetNumberOfTiles*channels);
                }
            }
            const auto fullTileBufferWidth = (lastTileX-firstTileX)*tileWidth;
            // Read tile to buffer
            auto tileBuffer = make_uninitialized_unique<uchar[]>(tileWidth*tileHeight*channels); // assume full tiles of same size for all
            for(auto tile : tilesToRead) {
                const int tileX = (tile.coord[0]-firstTileX)*tileWidth;
                const int tileY = (tile.coord[1]-firstTileY)*tileHeight;
                readVSITileToBuffer(tile, tileBuffer.get());
                // Stitch tile into full buffer
                for(int cy = 0; cy < tileHeight; ++cy) {
                    for(int cx = 0; cx < tileWidth; ++cx) {
                        for(int channel = 0; channel < channels; ++channel) {
                            fullTileBuffer[(tileX + cx + (tileY + cy)*fullTileBufferWidth)*channels + channel] = tileBuffer[(cx + cy*tileWidth)*channels + channel];
                        }
                    }
                }
            }

            // Crop the full buffer to data[]
            const int offsetX = x - firstTileX*tileWidth;
            const int offsetY = y - firstTileY*tileHeight;
            for(int cy = offsetY; cy < offsetY + height; ++cy) {
                for(int cx = offsetX; cx < offsetX + width; ++cx) {
                    for(int channel = 0; channel < channels; ++channel) {
                        data[(cx - offsetX + (cy - offsetY) * width)*channels + channel] = fullTileBuffer[(cx + cy * fullTileBufferWidth)*channels + channel];
                    }
                }
            }
        }
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
    try {
        level = m_image->getLevelForMagnification(magnification);
        reportInfo() << "Choose level " << level << " for image pyramid for magnification " << magnification << reportEnd();
    } catch(Exception &e) {
        // Magnification level not available
        // Have to sample for a higher level if possible
        reportWarning() << "Requested magnification level does not exist in image pyramid. " <<
                        "Will now try to sample from a lower level and resize. This may increase runtime." << reportEnd();
        // First find level which is larger than request magnification
        float targetSpacing = 0.00025f * (40.0f / (float)magnification);
        level = 0;
        float level0spacing = m_image->getSpacing().x();
        for(int i = 0; i < m_image->getNrOfLevels(); ++i) {
            float levelSpacing = m_image->getLevelScale(i)*level0spacing;
            level = i;
            resampleFactor = targetSpacing / levelSpacing; // Scale between level and the magnification level we want
            if(i+1 < m_image->getNrOfLevels() &&
               m_image->getLevelScale(i+1)*level0spacing > targetSpacing) {
                break;
            }
        }
        if(level < 0)
            throw Exception("Unable to get patch for magnification level " +
                            std::to_string(magnification) + " because level 0 was at a lower magnification ");
    }
    reportInfo() << "Extracting patch from level " << level << " and using a resampling factor of " << resampleFactor << reportEnd();

    // Convert offset from physical float position to pixel position in level
    offsetX = round((offsetX / m_image->getSpacing().x())/m_image->getLevelScale(level));
    offsetY = round((offsetY / m_image->getSpacing().y())/m_image->getLevelScale(level));
    std::cout << "Pixel offset " << offsetX << " " << offsetY << std::endl;

    const int patchWidth = width*resampleFactor;
    const int patchHeight = height*resampleFactor;

    auto image = getPatchAsImage(level, offsetX, offsetY, patchWidth, patchHeight, convertToRGB);

    if(resampleFactor > 1.0f) {
        image = ImageResizer::create(width, height)->connect(image)->runAndGetOutputData<Image>();
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
    } else {
        return writeTileToTIFF(level, x, y, data);
    }
}

uint32_t ImagePyramidAccess::writeTileToTIFF(int level, int x, int y, Image::pointer image) {
    if(m_image->getCompression() == ImageCompression::NEURAL_NETWORK) {
        return writeTileToTIFFNeuralNetwork(level, x, y, image);
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
    if(!propagate)
        return;
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
        tile_id = writeTileToTIFF(level, x, y, newData.get(), tileWidth, tileHeight, channels);
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
    std::lock_guard<std::mutex> lock(m_readMutex);
    TIFFSetDirectory(m_tiffHandle, level);
    auto tile = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    return m_initializedPatchList.count(std::to_string(level) + "-" + std::to_string(tile)) > 0;
}

int ImagePyramidAccess::readTileFromTIFF(void *data, int x, int y) {
    // Assumes level (directory is already set)
    if(m_compressionFormat == ImageCompression::NEURAL_NETWORK) {
        auto decompressionModel = m_image->getDecompressionModel();
        // TODO The logic here must be improved
        // TODO this assumes fixed size code
        auto shape = decompressionModel->getInputNodes().begin()->second.shape;
        shape[0] = 1;
        int64_t size = shape.getTotalSize()*4;
        float* buffer = new float[shape.getTotalSize()];
        uint32_t tile_id = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
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
        int bytesRead = TIFFReadTile(m_tiffHandle, data, x, y, 0, 0);
        return bytesRead;
    }
}


}
