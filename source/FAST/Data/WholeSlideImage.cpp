#include "WholeSlideImage.hpp"
#include <openslide/openslide.h>
#include <FAST/Utility.hpp>

namespace fast {

void WholeSlideImage::create(openslide_t *fileHandle, std::vector<WholeSlideImageLevel> levels) {
    m_fileHandle = fileHandle;
    m_levels = levels;
    for(int i = 0; i < m_levels.size(); ++i) {
        int x = m_levels.size() - i - 1;
        m_levels[i].tiles = x*x*x + 10;
        std::cout << "Level tiles " << i << " " << m_levels[i].tiles << std::endl;
    }
    mBoundingBox = BoundingBox(Vector3f(getFullWidth(), getFullHeight(), 0));
}

int WholeSlideImage::getNrOfLevels() {
    return m_levels.size();
}

int WholeSlideImage::getLevelWidth(uint level) {
    return m_levels[level].width;
}

int WholeSlideImage::getLevelHeight(uint level) {
    return m_levels[level].height;
}

int WholeSlideImage::getLevelTiles(uint level) {
    return m_levels[level].tiles;
}

int WholeSlideImage::getFullWidth() {
    return m_levels[0].width;
}

int WholeSlideImage::getFullHeight() {
    return m_levels[0].height;
}

void WholeSlideImage::free(ExecutionDevice::pointer device) {
    freeAll();
}

void WholeSlideImage::freeAll() {
    m_levels.clear();
    m_tileCache.clear();
    openslide_close(m_fileHandle);
}

WholeSlideImage::~WholeSlideImage() {
    freeAll();
}

WholeSlideImageTile WholeSlideImage::getTile(std::string tile) {
    if(m_tileCache.count(tile) > 0)
        return m_tileCache[tile];

    auto parts = split(tile, "_");
    if(parts.size() != 3)
        throw Exception("incorrect tile format");

    int level = std::stoi(parts[0]);
    int tile_x = std::stoi(parts[1]);
    int tile_y = std::stoi(parts[2]);

    return getTile(level, tile_x, tile_y);
}

WholeSlideImageTile WholeSlideImage::getTile(int level, int tile_x, int tile_y) {
    std::string tileStr = std::to_string(level) + "_" + std::to_string(tile_x) + "_" + std::to_string(tile_y);
    if(m_tileCache.count(tileStr) > 0)
        return m_tileCache[tileStr];

    // Create tile
    int levelWidth = getLevelWidth(level);
    int levelHeight = getLevelHeight(level);
    int tiles = getLevelTiles(level);
    WholeSlideImageTile tile;
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
    tile.data = std::shared_ptr<uchar[]>(new uchar[bytes]); // TODO use make_shared instead (C++20)
    float scale = (float)getFullWidth()/levelWidth;
    openslide_read_region(m_fileHandle, (uint32_t *) tile.data.get(), std::floor(tile.offsetX*scale), std::floor(tile.offsetY*scale), level, tile.width, tile.height);
    m_tileCacheMemoryUsage += bytes;
    std::cout << m_tileCacheMemoryUsage/(1024*1024) << " MB usage" << std::endl;
    m_tileCache[tileStr] = tile;

    return tile;
}

}
