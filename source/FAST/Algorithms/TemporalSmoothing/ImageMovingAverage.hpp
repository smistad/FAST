#pragma once

#include <FAST/ProcessObject.hpp>
#include <deque>

namespace fast {

class Image;

class FAST_EXPORT ImageMovingAverage : public ProcessObject {
    FAST_OBJECT(ImageMovingAverage)
    public:
        void setFrameCount(int frameCount);
        void setKeepDataType(bool keep);
        void reset();
    protected:
        ImageMovingAverage();
        void execute() override;
        void loadAttributes() override;

        int m_frameCount;
        bool m_keepDataType;
        SharedPointer<Image> m_memory;
        std::deque<SharedPointer<Image>> m_buffer;
};

}