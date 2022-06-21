#pragma once

#include <FAST/Visualization/VolumeRenderer/VolumeRenderer.hpp>
#include <FAST/Visualization/VolumeRenderer/TransferFunction.hpp>

namespace fast {

/**
 * @brief Renders 3D images/volumes using ray-casting and alpha blending.
 *
 * Rays are cast back to front, accumulating color along the way based on a provided TransferFunction
 *
 * @ingroup renderers
 * @sa TransferFunction
 */
class FAST_EXPORT AlphaBlendingVolumeRenderer : public VolumeRenderer {
    FAST_PROCESS_OBJECT(AlphaBlendingVolumeRenderer)
    public:
        FAST_CONSTRUCTOR(AlphaBlendingVolumeRenderer, TransferFunction, transferFunction, = TransferFunction())
        /**
         * Set transfer function to use during alpha blending
         *
         * @param transferFunction
         */
        void setTransferFunction(TransferFunction transferFunction);
    protected:
        void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight) override;
        TransferFunction m_transferFunction;

};

}