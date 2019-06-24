#include "WholeSlideImage.hpp"
#ifdef WIN32
#include <winbase.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace fast {

void WholeSlideImage::create(std::vector<WholeSlideImageLevel::pointer> levels) {
    m_levels = levels;
    mBoundingBox = BoundingBox(Vector3f(getFullWidth(), getFullHeight(), 0));
}


WholeSlideImageLevel::pointer WholeSlideImage::getLevel(uint level) {
    m_levels[level]->mutex.lock();
    return m_levels[level];
}

WholeSlideImageLevel::pointer WholeSlideImage::getLastLevel() {
    return getLevel(m_levels.size()-1);
}

WholeSlideImageLevel::pointer WholeSlideImage::getFirstLevel() {
    return getLevel(0);
}

WholeSlideImageLevel::pointer WholeSlideImage::tryToGetLevel(uint level) {
    if(!m_levels[level]->mutex.try_lock()) {
        return nullptr;
    }
    return m_levels[level];
}

WholeSlideImageLevel::pointer WholeSlideImage::tryToGetLastLevel() {
    return tryToGetLevel(m_levels.size()-1);
}

WholeSlideImageLevel::pointer WholeSlideImage::tryToGetFirstLevel() {
    return tryToGetLevel(0);
}

int WholeSlideImage::getNrOfLevels() {
    return m_levels.size();
}
int WholeSlideImage::getLevelWidth(uint level) {
    return m_levels[level]->width;
}

int WholeSlideImage::getLevelHeight(uint level) {
    return m_levels[level]->height;
}

int WholeSlideImage::getFullWidth() {
    return m_levels[m_levels.size()-1]->width;
}
int WholeSlideImage::getFullHeight() {
    return m_levels[m_levels.size()-1]->height;
}

void WholeSlideImage::free(ExecutionDevice::pointer device) {
    freeAll();
}

void WholeSlideImage::freeAll() {
    for(auto&& item : m_levels) {
        if(item->memoryMapped) {
#ifdef WIN32
            UnmapViewOfFile(item->data);
            CloseHandle(item->fileHandle);
#else
            munmap(item->data, item->width*item->height*4);
            close(item->fileHandle);
#endif
        } else {
            delete[] item->data;
        }
    }
    m_levels.clear();
}

WholeSlideImage::~WholeSlideImage() {
    freeAll();
}

}
