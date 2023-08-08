#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Performs normalized envelope detection and log compression on IQ data
 *
 * Inputs:
 * - 0: Image 2 channels float (IQ data)
 *
 * Outputs:
 * - 0: Image 1 channel float (beamspace data in dB) if convertToGrayscale == false,
 *      or Image 1 channel uint8 (beamspace data in grayscale) if convertToGrayscale == true
 *
 * @ingroup ultrasound
 */
class FAST_EXPORT EnvelopeAndLogCompressor : public ProcessObject {
    FAST_PROCESS_OBJECT(EnvelopeAndLogCompressor)
    public:
        FAST_CONSTRUCTOR(EnvelopeAndLogCompressor,
                         bool, convertToGrayscale, = false,
                         float, gain, = 0,
                         float, dynamicRange, = 60
        )
        void setConvertToGrayscale(bool convert);
        void setGain(float gain);
        void setDynamicRange(float dynamicRange);
    private:
        void execute() override;

        float m_maxValue;
        bool m_maxInitialize = false;
        bool m_convertToGrayscale = false;
        float m_gain = 0;
        float m_dynamicRange = 60;
};
}