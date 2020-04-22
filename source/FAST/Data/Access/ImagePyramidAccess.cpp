#include "ImagePyramidAccess.hpp"
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Algorithms/ImageChannelConverter/ImageChannelConverter.hpp>
#include <FAST/Utility.hpp>
#include <openslide/openslide.h>
#include <FAST/Data/Image.hpp>

namespace fast {

ImagePyramidAccess::ImagePyramidAccess(std::vector<ImagePyramidLevel> levels, openslide_t* fileHandle, SharedPointer<ImagePyramid> imagePyramid, bool write) {
	if(levels.size() == 0)
		throw Exception("Image pyramid has no levels");
	m_image = imagePyramid;
	m_levels = levels;
	m_write = write;
    m_fileHandle = fileHandle;
}

void ImagePyramidAccess::release() {
	m_image->accessFinished();
}

ImagePyramidAccess::~ImagePyramidAccess() {
	release();
}

void ImagePyramidAccess::setScalar(uint x, uint y, uint level, uint8_t value, uint channel) {
	// Make sure it has write rights
	if(!m_write)
		throw Exception("ImagePyramidAccess has not write rights, but tried to write a value");
	auto levelData = m_levels[level];
	if(x >= levelData.width || y >= levelData.height)
		throw OutOfBoundsException();

	levelData.data[(x + y * levelData.width) * m_image->getNrOfChannels() + channel] = value;
}

uint8_t ImagePyramidAccess::getScalar(uint x, uint y, uint level, uint channel) {
	auto levelData = m_levels[level];
	if(x >= levelData.width || y >= levelData.height)
		throw OutOfBoundsException();
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

ImagePyramidPatch ImagePyramidAccess::getPatch(int level, int tile_x, int tile_y) {
    // Create patch
    int levelWidth = m_image->getLevelWidth(level);
    int levelHeight = m_image->getLevelHeight(level);
    int tiles = m_image->getLevelPatches(level);
    ImagePyramidPatch tile;
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
    float scale = (float)m_image->getFullWidth()/levelWidth;
    openslide_read_region(m_fileHandle, (uint32_t *) tile.data.get(), tile.offsetX*scale, tile.offsetY*scale, level, tile.width, tile.height);

    return tile;
}

SharedPointer<Image> ImagePyramidAccess::getLevelAsImage(int level) {
    if(level < 0 || level >= m_image->getNrOfLevels())
        throw Exception("Incorrect level given to getLevelAsImage" + std::to_string(level));

    int width = m_image->getLevelWidth(level);
    int height = m_image->getLevelHeight(level);
    if(width > 16384 || height > 16384)
        throw Exception("Image level is too large to convert into a FAST image");

    auto image = Image::New();
    auto data = make_uninitialized_unique<uchar[]>(width*height*4);
    openslide_read_region(m_fileHandle, (uint32_t *)data.get(), 0, 0, level, width, height);
    image->create(width, height, TYPE_UINT8, 4, std::move(data));
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

SharedPointer<Image> ImagePyramidAccess::getPatchAsImage(int level, int offsetX, int offsetY, int width, int height) {
    if(width > 16384 || height > 16384)
        throw Exception("Image level is too large to convert into a FAST image");

    if(offsetX < 0 || offsetY < 0 || width <= 0 || height <= 0)
        throw Exception("Offset and size must be positive");

    if(offsetX + width >= m_image->getLevelWidth(level) || offsetY + height >= m_image->getLevelHeight(level))
        throw Exception("offset + size exceeds level size");

    auto image = Image::New();
    auto data = make_uninitialized_unique<uchar[]>(width*height*4);
    float scale = (float)m_image->getFullWidth()/m_image->getLevelWidth(level);
    openslide_read_region(m_fileHandle, (uint32_t *)data.get(), offsetX*scale, offsetY*scale, level, width, height);
    image->create(width, height, TYPE_UINT8, 4, std::move(data));
    image->setSpacing(Vector3f(
            scale,
            scale,
            1.0f
    ));
    // TODO Set transformation
    SceneGraph::setParentNode(image, std::dynamic_pointer_cast<SpatialDataObject>(m_image));

    // Data is stored as BGRA, need to delete alpha channel and reverse it
    auto channelConverter = ImageChannelConverter::New();
    channelConverter->setChannelsToRemove(false, false, false, true);
    channelConverter->setReverseChannels(true);
    channelConverter->setInputData(image);

    return channelConverter->updateAndGetOutputData<Image>();
}



}