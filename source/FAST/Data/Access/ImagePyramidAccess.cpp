#include "ImagePyramidAccess.hpp"
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Algorithms/ImageChannelConverter/ImageChannelConverter.hpp>
#include <FAST/Utility.hpp>
#include <openslide/openslide.h>
#include <tiffio.h>
#include <FAST/Data/Image.hpp>
#include <jpeglib.h>

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

ImagePyramidPatch ImagePyramidAccess::getPatch(std::string tile) {
    auto parts = split(tile, "_");
    if(parts.size() != 3)
        throw Exception("incorrect tile format");

    int level = std::stoi(parts[0]);
    int tile_x = std::stoi(parts[1]);
    int tile_y = std::stoi(parts[2]);

    return getPatch(level, tile_x, tile_y);
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

std::unique_ptr<uchar[]> ImagePyramidAccess::getPatchData(int level, int x, int y, int width, int height) {
    const int levelWidth = m_image->getLevelWidth(level);
    const int levelHeight = m_image->getLevelHeight(level);
    const int channels = m_image->getNrOfChannels();
    const int tileWidth = m_image->getLevelTileWidth(level);
    const int tileHeight = m_image->getLevelTileHeight(level);
    auto data = std::make_unique<uchar[]>(width*height*channels);
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
            int bytesRead = TIFFReadTile(m_tiffHandle, (void *) data.get(), x, y, 0, 0);
        } else if((width < tileWidth || height < tileHeight) && x % tileWidth == 0 && y % tileHeight == 0) {
            auto tileData = std::make_unique<uchar[]>(tileWidth*tileHeight*channels);
            {
                // From TIFFReadTile documentation: Return the data for the tile containing the specified coordinates.
                // In TIFF all tiles have the same size, thus they are padded..
                int bytesRead = TIFFReadTile(m_tiffHandle, (void *) tileData.get(), x, y, 0, 0);
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
            auto fullTileBuffer = make_uninitialized_unique<uchar[]>(tileWidth*tileHeight*targetNumberOfTiles*channels);
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
                    auto tileData = std::make_unique<uchar[]>(tileWidth*tileHeight*channels);
                    int tileX = i*tileWidth;
                    int tileY = j*tileHeight;
                    int bytesRead = TIFFReadTile(m_tiffHandle, (void *) tileData.get(), firstTileX*tileWidth+tileX, firstTileY*tileHeight+tileY, 0, 0);
                    // Stitch tile into full buffer
                    for(int cy = 0; cy < tileHeight; ++cy) {
                        for(int cx = 0; cx < tileWidth; ++cx) {
                            for(int channel = 0; channel < channels; ++channel) {
                                fullTileBuffer[(tileX + cx + (tileY + cy)*fullTileBufferWidth)*channels + channel] = tileData[(cx + cy*tileWidth)*channels + channel];
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
                        data[(cx - offsetX + (cy - offsetY) * width)*channels + channel] = fullTileBuffer[(cx + cy * fullTileBufferWidth)*channels + channel];
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

    return getPatchAsImage(level, 0, 0, width, height, true);
}

std::shared_ptr<Image> ImagePyramidAccess::getPatchAsImage(int level, int offsetX, int offsetY, int width, int height, bool convertToRGB) {
    if(width > 16384 || height > 16384)
        throw Exception("Image level is too large to convert into a FAST image");

    if(offsetX < 0 || offsetY < 0 || width <= 0 || height <= 0)
        throw Exception("Offset and size must be positive");

    if(offsetX + width > m_image->getLevelWidth(level) || offsetY + height > m_image->getLevelHeight(level))
        throw Exception("offset + size exceeds level size");

    auto data = getPatchData(level, offsetX, offsetY, width, height);
    float scale = m_image->getLevelScale(level);
    auto spacing = m_image->getSpacing();
    auto image = Image::create(width, height, TYPE_UINT8, m_image->getNrOfChannels(), std::move(data));
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

    // Read the actual data
    auto data = getPatchData(level, tile.offsetX, tile.offsetY, tile.width, tile.height);

    float scale = m_image->getLevelScale(level);
    Vector3f spacing = m_image->getSpacing();
    auto image = Image::create(tile.width, tile.height, TYPE_UINT8, m_image->getNrOfChannels(), std::move(data));
    image->setSpacing(Vector3f(
            scale*spacing.x(),
            scale*spacing.y(),
            1.0f
    ));
    // TODO Set transformation
    SceneGraph::setParentNode(image, std::dynamic_pointer_cast<SpatialDataObject>(m_image));

    if(m_fileHandle != nullptr && convertToRGB) {
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
    auto patchAccess = patch->getImageAccess(ACCESS_READ);
    auto data = (uchar*)patchAccess->get();
    uint32_t tile_id;
    {
        std::lock_guard<std::mutex> lock(m_readMutex);
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

    // Propagate upwards
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
        {
            std::lock_guard<std::mutex> lock(m_readMutex);
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
    std::lock_guard<std::mutex> lock(m_readMutex);
    TIFFSetDirectory(m_tiffHandle, level);
    auto tile = TIFFComputeTile(m_tiffHandle, x, y, 0, 0);
    return m_initializedPatchList.count(std::to_string(level) + "-" + std::to_string(tile)) > 0;
}

}
