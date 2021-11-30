#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT ScanConverter : public ProcessObject {
    FAST_PROCESS_OBJECT(ScanConverter)
    public:
        FAST_CONSTRUCTOR(ScanConverter,
                         int, width, = 1024,
                         int, height, = 1024,
                         float, gain, = 10.0f,
                         float, dynamicRange, = 60.0f
        )
    private:
        void execute() override;

        int m_width;
        int m_height;
        float m_gain;
        float m_dynamicRange;
};

}