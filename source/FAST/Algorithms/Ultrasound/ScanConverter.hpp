#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Scan convert beamspace image
 *
 * Inputs:
 * - 0: Beamspace image (float) in dB (typically from EnvelopeAndLogCompressor)
 *
 * Outputs:
 * - 0: Scan converted grayscale image (uint8)
 *
 * @ingroup ultrasound
 */
class FAST_EXPORT ScanConverter : public ProcessObject {
    FAST_PROCESS_OBJECT(ScanConverter)
    public:
        FAST_CONSTRUCTOR(ScanConverter,
                         int, width, = 1024,
                         int, height, = 1024,
                         float, gain, = 10.0f,
                         float, dynamicRange, = 60.0f
        );
        void setWidth(int width);
        int getWidth() const;
        void setHeight(int height);
        int getHeight() const;
        void setGain(float gain);
        void setDynamicRange(float dynamicRange);
    private:
        void execute() override;

        int m_width;
        int m_height;
        float m_gain;
        float m_dynamicRange;
};

}