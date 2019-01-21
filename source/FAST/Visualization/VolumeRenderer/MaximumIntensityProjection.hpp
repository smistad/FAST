#pragma once

#include <FAST/Visualization/VolumeRenderer/VolumeRenderer.hpp>

namespace fast {

class FAST_EXPORT MaximumIntensityProjection : public VolumeRenderer {
    FAST_OBJECT(MaximumIntensityProjection)
    public:
    protected:
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D) override;
    private:
        MaximumIntensityProjection();
};

}