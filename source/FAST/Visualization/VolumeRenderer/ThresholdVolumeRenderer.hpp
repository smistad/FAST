#pragma once

#include <FAST/Visualization/VolumeRenderer/VolumeRenderer.hpp>

namespace fast {

class FAST_EXPORT ThresholdVolumeRenderer : public VolumeRenderer {
    FAST_OBJECT(ThresholdVolumeRenderer)
    public:
        void setThreshold(float threshold);
    protected:
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D) override;
        float m_threshold = 0;
    private:
        ThresholdVolumeRenderer();
};

}