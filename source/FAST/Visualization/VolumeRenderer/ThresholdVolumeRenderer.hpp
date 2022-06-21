#pragma once

#include <FAST/Visualization/VolumeRenderer/VolumeRenderer.hpp>

namespace fast {

/**
 * @brief Renders 3D images using ray-casting and a threshold
 *
 * @ingroup renderers
 */
class FAST_EXPORT ThresholdVolumeRenderer : public VolumeRenderer {
    FAST_PROCESS_OBJECT(ThresholdVolumeRenderer)
    public:
        FAST_CONSTRUCTOR(ThresholdVolumeRenderer, float, threshold, = 0)
        void setThreshold(float threshold);
    protected:
        void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight) override;
        float m_threshold = 0;
};

}