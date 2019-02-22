#pragma once

#include <FAST/Visualization/VolumeRenderer/VolumeRenderer.hpp>
#include <FAST/Visualization/VolumeRenderer/TransferFunction.hpp>

namespace fast {

class FAST_EXPORT AlphaBlendingVolumeRenderer : public VolumeRenderer {
    FAST_OBJECT(AlphaBlendingVolumeRenderer)
    public:
        void setTransferFunction(TransferFunction transferFunction);
    protected:
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) override;
        TransferFunction m_transferFunction;
    private:
        AlphaBlendingVolumeRenderer();

};

}