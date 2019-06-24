#include "WholeSlideImage.hpp"
#include <openslide/openslide.h>
#include <FAST/Utility.hpp>
#include <thread>

namespace fast {

void WholeSlideImage::create(openslide_t *fileHandle, std::vector<WholeSlideImageLevel> levels) {
    m_fileHandle = fileHandle;
    m_levels = levels;
    for(int i = 0; i < m_levels.size(); ++i) {
        int x = m_levels.size() - i - 1;
        m_levels[i].tiles = x*x*x + 10;
        std::cout << "Level tiles " << i << " " << m_levels[i].tiles << std::endl;
    }
    // Cache entire first level
    int tiles = m_levels[m_levels.size()-1].tiles;
    for(int tile_x = 0; tile_x < tiles; ++tile_x) {
        for(int tile_y = 0; tile_y < tiles; ++tile_y) {
            cacheTiles(m_levels.size(), (float)tile_x/tiles, (float)tile_y/tiles);
        }
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

WholeSlideImageTile WholeSlideImage::getTile(std::string tile, bool okWithLowRes) {
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        if(m_tileCache.count(tile) > 0)
            return m_tileCache[tile];
    }


    auto parts = split(tile, "_");
    if(parts.size() != 3)
        throw Exception("incorrect tile format");

    int level = std::stoi(parts[0]);
    int tile_x = std::stoi(parts[1]);
    int tile_y = std::stoi(parts[2]);

    return getTile(level, tile_x, tile_y, okWithLowRes);
}

void WholeSlideImage::cacheTiles(int level, float normalizedPositionX, float normalizedPositionY) {
    int levelBelow = level - 1;
    if(levelBelow < 0)
        levelBelow = 0;
    int levelAbove = level + 1;
    if(levelAbove >= getNrOfLevels())
        levelAbove = getNrOfLevels()-1;

    int tile_x, tile_y;
    tile_x = round(getLevelTiles(levelBelow)*normalizedPositionX);
    tile_y = round(getLevelTiles(levelBelow)*normalizedPositionY);
    getTile(levelBelow, tile_x, tile_y, false);

    tile_x = round(getLevelTiles(levelAbove)*normalizedPositionX);
    tile_y = round(getLevelTiles(levelAbove)*normalizedPositionY);
    getTile(levelAbove, tile_x, tile_y, false);
}

WholeSlideImageTile WholeSlideImage::getTileNormalized(int level, float normalizedPositionX, float normalizedPositionY) {
    int tile_x, tile_y;
    tile_x = round(getLevelTiles(level)*normalizedPositionX);
    tile_y = round(getLevelTiles(level)*normalizedPositionY);
    std::cout << "Getting normalized tile" << level << ": " << tile_x << tile_y << std::endl;
    getTile(level, tile_x, tile_y, false);
}

WholeSlideImageTile WholeSlideImage::getTile(int level, int tile_x, int tile_y, bool okWithLowRes) {
    int levelWidth = getLevelWidth(level);
    int levelHeight = getLevelHeight(level);

    // Create tile
    int tiles = getLevelTiles(level);
    WholeSlideImageTile tile;
    tile.offsetX = tile_x * (int) std::floor((float) levelWidth / tiles);
    tile.offsetY = tile_y * (int) std::floor((float) levelHeight / tiles);

    {
        std::unique_lock<std::mutex> lock(m_cacheMutex);
        std::string tileStr = std::to_string(level) + "_" + std::to_string(tile_x) + "_" + std::to_string(tile_y);
        if(m_tileCache.count(tileStr) > 0) {
            return m_tileCache[tileStr];
        } else if(okWithLowRes) {
            lock.unlock();
            return getTileNormalized(level+1, (float)tile_x/levelWidth, (float)tile_y/levelHeight);
        }

        tile.width = std::floor(((float) levelWidth / tiles));
        if(tile_x == tiles - 1)
            tile.width = levelWidth - tile.offsetX;
        tile.height = std::floor((float) levelHeight / tiles);
        if(tile_y == tiles - 1)
            tile.height = levelHeight - tile.offsetY;

        //std::cout << "Loading data from disk for tile " << tile_x << " " << tile_y << " " << level << " " <<  tile_offset_x << " " << tile_offset_y << std::endl;
        // Read the actual data
        std::size_t bytes = tile.width * tile.height * 4;
        tile.data = std::shared_ptr<uchar[]>(new uchar[bytes]); // TODO use make_shared instead (C++20)
        float scale = (float) getFullWidth() / levelWidth;
        openslide_read_region(m_fileHandle, (uint32_t *) tile.data.get(), std::floor(tile.offsetX * scale),
                              std::floor(tile.offsetY * scale), level, tile.width, tile.height);
        m_tileCacheMemoryUsage += bytes;
        std::cout << m_tileCacheMemoryUsage / (1024 * 1024) << " MB usage" << std::endl;
        m_tileCache[tileStr] = tile;
    }

    // TODO cache neighboring tiles
    // Cache tile below async
    /*
    float normalizedPositionX = (float)tile.offsetX/levelWidth;
    float normalizedPositionY = (float)tile.offsetY/levelHeight;
    std::thread([this, level, normalizedPositionX, normalizedPositionY](){
        cacheTiles(level, normalizedPositionX, normalizedPositionY);
    }).detach();
     */

    return tile;
}

}
