#pragma once

#include <FAST/ProcessObject.hpp>
#include <deque>

namespace fast {

class Image;

/**
 * @brief Temporal smoothing of image using moving average
 *
 * Inputs:
 * - 0: Image stream
 *
 * Outputs:
 * - 0: Image stream
 *
 * @ingroup filter
 */
class FAST_EXPORT ImageMovingAverage : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageMovingAverage)
    public:
        FAST_CONSTRUCTOR(ImageMovingAverage,
                         int, frameCount, = 10,
                         bool, keepDataType, = false
        )
        void setFrameCount(int frameCount);
        void setKeepDataType(bool keep);
        void reset();
    protected:
        void execute() override;
        void loadAttributes() override;

        int m_frameCount;
        bool m_keepDataType;
        std::shared_ptr<Image> m_memory;
        std::deque<std::shared_ptr<Image>> m_buffer;
};

}