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
 * @sa EnvelopeAndLogCompressor
 * @sa UFFStreamer
 */
class FAST_EXPORT ScanConverter : public ProcessObject {
    FAST_PROCESS_OBJECT(ScanConverter)
    public:
    /**
     * @brief Create instance
     *
     * The scan converter parameters (start/end depth, start/end angle etc.) can be set manually here.
     * But this is only needed if the input image frame lacks this information in its frame data.
     * The UFFStreamer should provide this information.
     *
     * @param width Width of output image
     * @param height Height of output image
     * @param gain Set gain to be used if input is in dB
     * @param dynamicRange Set dynamic range to be used if input is in dB
     * @param startDepth Start depth scan in millimeters.
     *      This is only needed to be set manually if the input frame lacks this information in its frame data.
     * @param endDepth End depth of scan in millimeters.
     *      This is only needed to be set manually if the input frame lacks this information in its frame data.
     * @param startAngle Start angle of sector scan in radians. Not needed if linear array image.
     *      This is only needed to be set manually if the input frame lacks this information in its frame data.
     * @param endAngle End angle of sector scan in radians. Not needed if linear array image.
     *      This is only needed to be set manually if the input frame lacks this information in its frame data.
     * @param left Lateral start position of linear scan in millimeters. Not needed if sector scan.
     *      This is only needed to be set manually if the input frame lacks this information in its frame data.
     * @param right Lateral end position of linear scan in millimeters. Not needed if sector scan.
     *      This is only needed to be set manually if the input frame lacks this information in its frame data.
     * @return instance
     */
        FAST_CONSTRUCTOR(ScanConverter,
                         int, width, = 1024,
                         int, height, = 1024,
                         float, gain, = 10.0f,
                         float, dynamicRange, = 60.0f,
                         float, startDepth, = 0.0f,
                         float, endDepth, = 0.0f,
                         float, startAngle, = 0.0f,
                         float, endAngle, = 0.0f,
                         float, left, = 0.0f,
                         float, right, = 0.0f
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

        float m_startDepth;
        float m_endDepth;
        float m_startAngle;
        float m_endAngle;
        float m_leftPos;
        float m_rightPos;
        float m_depthSpacing;
        float m_lateralSpacing;
};

}