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
 * - 0: Image 1 channel float (beamspace data in dB)
 *
 * @ingroup ultrasound
 */
class FAST_EXPORT EnvelopeAndLogCompressor : public ProcessObject {
    FAST_PROCESS_OBJECT(EnvelopeAndLogCompressor)
    public:
        FAST_CONSTRUCTOR(EnvelopeAndLogCompressor)
    private:
        void execute() override;

        float m_maxValue;
        bool m_maxInitialize = false;
};
}