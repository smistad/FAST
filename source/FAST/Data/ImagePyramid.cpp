#include "ImagePyramid.hpp"
#include <openslide/openslide.h>
#include <FAST/Utility.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/ImageChannelConverter/ImageChannelConverter.hpp>

namespace fast {

void ImagePyramid::create(openslide_t *fileHandle, std::vector<ImagePyramid::Level> levels) {
    m_fileHandle = fileHandle;
    m_levels = levels;
    for(int i = 0; i < m_levels.size(); ++i) {
        int x = m_levels.size() - i - 1;
        m_levels[i].patches = x*x*x + 10;
    }
    mBoundingBox = BoundingBox(Vector3f(getFullWidth(), getFullHeight(), 0));
}

int ImagePyramid::getNrOfLevels() {
    return m_levels.size();
}

int ImagePyramid::getLevelWidth(int level) {
    return m_levels.at(level).width;
}

int ImagePyramid::getLevelHeight(int level) {
    return m_levels.at(level).height;
}

int ImagePyramid::getLevelPatches(int level) {
    return m_levels.at(level).patches;
}

int ImagePyramid::getFullWidth() {
    return m_levels[0].width;
}

int ImagePyramid::getFullHeight() {
    return m_levels[0].height;
}

void ImagePyramid::free(ExecutionDevice::pointer device) {
    freeAll();
}

void ImagePyramid::freeAll() {
    m_levels.clear();
    openslide_close(m_fileHandle);
}

ImagePyramid::~ImagePyramid() {
    freeAll();
}

ImagePyramid::Patch ImagePyramid::getPatch(std::string tile) {
    auto parts = split(tile, "_");
    if(parts.size() != 3)
        throw Exception("incorrect tile format");

    int level = std::stoi(parts[0]);
    int tile_x = std::stoi(parts[1]);
    int tile_y = std::stoi(parts[2]);

    return getPatch(level, tile_x, tile_y);
}

ImagePyramid::Patch ImagePyramid::getPatch(int level, int tile_x, int tile_y) {
    // Create patch
    int levelWidth = getLevelWidth(level);
    int levelHeight = getLevelHeight(level);
    int tiles = getLevelPatches(level);
    ImagePyramid::Patch tile;
    tile.offsetX = tile_x * (int) std::floor((float) levelWidth / tiles);
    tile.offsetY = tile_y * (int) std::floor((float) levelHeight / tiles);

    tile.width = std::floor(((float) levelWidth / tiles));
    if(tile_x == tiles - 1)
        tile.width = levelWidth - tile.offsetX;
    tile.height = std::floor((float) levelHeight / tiles);
    if(tile_y == tiles - 1)
        tile.height = levelHeight - tile.offsetY;

    //std::cout << "Loading data from disk for tile " << tile_x << " " << tile_y << " " << level << " " <<  tile_offset_x << " " << tile_offset_y << std::endl;
    // Read the actual data
    std::size_t bytes = tile.width*tile.height*4;
    tile.data = std::shared_ptr<uchar>(new uchar[bytes], [](uchar *p) { delete [] p; }); // TODO use make_shared instead (C++20)
    float scale = (float)getFullWidth()/levelWidth;
    openslide_read_region(m_fileHandle, (uint32_t *) tile.data.get(), tile.offsetX*scale, tile.offsetY*scale, level, tile.width, tile.height);

    return tile;
}

SharedPointer<Image> ImagePyramid::getLevelAsImage(int level) {
    if(level < 0 || level >= getNrOfLevels())
        throw Exception("Incorrect level given to getLevelAsImage" + std::to_string(level));

    int width = getLevelWidth(level);
    int height = getLevelHeight(level);
    if(width > 16384 || height > 16384)
        throw Exception("Image level is too large to convert into a FAST image");

    auto image = Image::New();
    auto data = make_uninitialized_unique<uchar[]>(width*height*4);
    openslide_read_region(m_fileHandle, (uint32_t *)data.get(), 0, 0, level, width, height);
    image->create(width, height, TYPE_UINT8, 4, std::move(data));
    image->setSpacing(Vector3f(
            (float)getFullWidth() / width,
            (float)getFullHeight() / height,
            1.0f
    ));
    SceneGraph::setParentNode(image, std::dynamic_pointer_cast<SpatialDataObject>(mPtr.lock()));

    // Data is stored as BGRA, need to delete alpha channel and reverse it
    auto channelConverter = ImageChannelConverter::New();
    channelConverter->setChannelsToRemove(false, false, false, true);
    channelConverter->setReverseChannels(true);
    channelConverter->setInputData(image);
    auto port = channelConverter->getOutputPort();
    channelConverter->update();

    return port->getNextFrame<Image>();
}

SharedPointer<Image> ImagePyramid::getPatchAsImage(int level, int offsetX, int offsetY, int width, int height) {
    if(width > 16384 || height > 16384)
        throw Exception("Image level is too large to convert into a FAST image");

    if(offsetX < 0 || offsetY < 0 || width <= 0 || height <= 0)
        throw Exception("Offset and size must be positive");

    if(offsetX + width >= getLevelWidth(level) || offsetY + height >= getLevelHeight(level))
        throw Exception("offset + size exceeds level size");

    auto image = Image::New();
    auto data = make_uninitialized_unique<uchar[]>(width*height*4);
    float scale = (float)getFullWidth()/getLevelWidth(level);
    openslide_read_region(m_fileHandle, (uint32_t *)data.get(), offsetX*scale, offsetY*scale, level, width, height);
    image->create(width, height, TYPE_UINT8, 4, std::move(data));
    image->setSpacing(Vector3f(
            scale,
            scale,
            1.0f
    ));
    // TODO Set transformation
    SceneGraph::setParentNode(image, std::dynamic_pointer_cast<SpatialDataObject>(mPtr.lock()));

    // Data is stored as BGRA, need to delete alpha channel and reverse it
    auto channelConverter = ImageChannelConverter::New();
    channelConverter->setChannelsToRemove(false, false, false, true);
    channelConverter->setReverseChannels(true);
    channelConverter->setInputData(image);
    auto port = channelConverter->getOutputPort();
    channelConverter->update();

    return port->getNextFrame<Image>();
}

}
