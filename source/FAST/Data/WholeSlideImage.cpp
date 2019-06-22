#include "WholeSlideImage.hpp"
#ifdef WIN32
#include <winbase.h>
#else
#include <sys/mman.h>
#endif

namespace fast {

void WholeSlideImage::create(std::vector<WholeSlideImageLevel> levels) {
    m_levels = levels;
    int width = levels.back().width;
    int height = levels.back().height;
    mBoundingBox = BoundingBox(Vector3f(width, height, 0));
}

void WholeSlideImage::free(ExecutionDevice::pointer device) {
    freeAll();
}

void WholeSlideImage::freeAll() {
    for(auto&& item : m_levels) {
        if(item.memoryMapped) {
#ifdef WIN32
            UnmapViewOfFile(item.data);
            CloseHandle(item.fileHandle);
#else
            munmap(item.data, item.width*item.height*4);
            close(item.fileHandle);
#endif
        } else {
            delete[] item.data;
        }
    }
    m_levels.clear();
}

WholeSlideImage::~WholeSlideImage() {
    freeAll();
}

}
