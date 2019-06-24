#pragma once

#include "SpatialDataObject.hpp"

namespace fast {

class FAST_EXPORT WholeSlideImageLevel : public Object {
    FAST_OBJECT(WholeSlideImageLevel)
    public:
        int width;
        int height;
        uint8_t* data;
        bool memoryMapped;
        std::mutex mutex;
#ifdef WIN32
        void* fileHandle;
#else
        int fileHandle;
#endif
};

class FAST_EXPORT WholeSlideImage : public SpatialDataObject {
    FAST_OBJECT(WholeSlideImage)
    public:
        void create(std::vector<WholeSlideImageLevel::pointer> levels);
        WholeSlideImageLevel::pointer getLevel(uint level);
        WholeSlideImageLevel::pointer getLastLevel();
        WholeSlideImageLevel::pointer getFirstLevel();
        WholeSlideImageLevel::pointer tryToGetLevel(uint level);
        WholeSlideImageLevel::pointer tryToGetLastLevel();
        WholeSlideImageLevel::pointer tryToGetFirstLevel();
        int getNrOfLevels();
        int getLevelWidth(uint level);
        int getLevelHeight(uint level);
        int getFullWidth();
        int getFullHeight();
        void free(ExecutionDevice::pointer device) override;
        void freeAll() override;
        ~WholeSlideImage();
    private:
        std::vector<WholeSlideImageLevel::pointer> m_levels;
};

}
