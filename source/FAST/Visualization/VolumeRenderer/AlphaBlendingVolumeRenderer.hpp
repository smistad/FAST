#pragma once

#include <FAST/Visualization/VolumeRenderer/VolumeRenderer.hpp>
#include <FAST/Visualization/VolumeRenderer/TransferFunction.hpp>

namespace fast {

/**
 * Ray-casting based volume rendering using alpha blending.
 * Rays are cast back to front, accumulating color along the way based on a provided transfer function.
 */
class FAST_EXPORT AlphaBlendingVolumeRenderer : public VolumeRenderer {
    FAST_OBJECT(AlphaBlendingVolumeRenderer)
    public:
        /**
         * Set transfer function to use during alpha blending
         *
         * @param transferFunction
         */
        void setTransferFunction(TransferFunction transferFunction);
    protected:
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) override;
        TransferFunction m_transferFunction;
    private:
        AlphaBlendingVolumeRenderer();

};

}