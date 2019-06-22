#pragma once

#include "SpatialDataObject.hpp"

namespace fast {

struct WholeSlideImageLevel {
    int width;
    int height;
    uint8_t* data;
    bool memoryMapped;
    void* fileHandle;
};

class FAST_EXPORT WholeSlideImage : public SpatialDataObject {
    FAST_OBJECT(WholeSlideImage)
    public:
        void create(std::vector<WholeSlideImageLevel> levels);
        std::vector<WholeSlideImageLevel> m_levels;
        void free(ExecutionDevice::pointer device) override;
        void freeAll() override;
        ~WholeSlideImage();
};

}
