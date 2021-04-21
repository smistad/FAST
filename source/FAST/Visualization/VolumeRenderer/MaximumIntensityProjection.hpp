#pragma once

#include <FAST/Visualization/VolumeRenderer/VolumeRenderer.hpp>

namespace fast {

/**
 * @brief Renders 3D images using ray-casting and maximum intensity projection (MIP)
 *
 * Uses the highest intensity observed along the ray.
 *
 * @ingroup renderers
 */
class FAST_EXPORT MaximumIntensityProjection : public VolumeRenderer {
    FAST_OBJECT(MaximumIntensityProjection)
    public:
    protected:
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) override;
    private:
        MaximumIntensityProjection();
};

}