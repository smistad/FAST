#pragma once

#include <FAST/Visualization/Plane.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class ImageSlicer;

/**
 * @brief Renders a 2D image slice extracted from a 3D image.
 *
 * This renderer uses the ImageSlicer to extract a 2D image slice from a 3D image, and
 * extends ImageRenderer to render the image slice.
 *
 * @ingroup renderers
 */
class FAST_EXPORT  SliceRenderer : public ImageRenderer {
    FAST_OBJECT(SliceRenderer)
    public:
        uint addInputConnection(DataChannel::pointer port) override;
        /**
         * Orthogonal slicing using the specified orthogonal plane.
         * @param port
         * @param orthogonalSlicePlane
         * @param sliceNr
         * @return port id of new port
         */
        uint addInputConnection(DataChannel::pointer port, PlaneType orthogonalSlicePlane, int sliceNr = -1);
        /**
         * Arbitrary slicing using the supplied plane.
         * @param port
         * @param slicePlane
         * @return port id of new port
         */
        uint addInputConnection(DataChannel::pointer port, Plane slicePlane);
        /**
         * Add new input connection using supplied slicer
         * @param port
         * @param slicer
         * @return port id of new port
         */
        uint addInputConnection(DataChannel::pointer port, std::shared_ptr<ImageSlicer> slicer);
        void setOrthogonalSlicePlane(uint portID, PlaneType orthogonalSlicePlane, int sliceNr = -1);
        void setArbitrarySlicePlane(uint portID, Plane slicePlane);
    private:
        SliceRenderer();
        void execute() override;

        std::unordered_map<uint, std::shared_ptr<ImageSlicer>> mSlicers;
};

}
