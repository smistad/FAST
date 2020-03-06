#pragma once

#include <FAST/ProcessObject.hpp>
#include <queue>

namespace fast {

class Image;

class FAST_EXPORT ImageWeightedMovingAverage : public ProcessObject {
    FAST_OBJECT(ImageWeightedMovingAverage)
    public:
        void setFrameCount(int frameCount);
    protected:
        ImageWeightedMovingAverage();
        void execute() override;
        void loadAttributes() override;

        int m_frameCount;
        SharedPointer<Image> m_memory;
        std::queue<SharedPointer<Image>> m_buffer;
};

}