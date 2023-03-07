#pragma once

#include <FAST/Visualization/Window.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/SegmentationLabelRenderer/SegmentationLabelRenderer.hpp>
#include <FAST/Algorithms/ImageSlicer/ImageSlicer.hpp>

class QSlider;

namespace fast {

/**
 * @brief Window for showing slices of 3D data
 *
 * This window shows 3 views with slices of the volume data in three different directions (X, Y, Z).
 * This windows features a slider for each view which controls which slice of the volume is being displayed.
 * It also has a widget for controlling intensity window and level which is often needed for CT and MRI data.
 * Use the SlicerWindow::connectImage and SlicerWindow::connectSegmentation methods to connect data to this window.
 * The window will setup the appropriate renderers for you.
 *
 * @todo Improve segmentation label rendering
 * @todo Support for image and segmentations with different sizes
 * @ingroup window
 */
class FAST_EXPORT SlicerWindow : public Window {
    FAST_OBJECT_V4(SlicerWindow)

    public:
        FAST_CONSTRUCTOR(SlicerWindow,
                         Color, bgcolor, = Color::Black(),
                         uint, width, = 0,
                         uint, height, = 0
        );
        std::shared_ptr<SlicerWindow> connectImage(std::shared_ptr<ProcessObject> processObject, float level = -1, float window = -1);
        std::shared_ptr<SlicerWindow> connectSegmentation(std::shared_ptr<ProcessObject> processObject,
                                                          LabelColors colors = LabelColors(),
                                                          float opacity = 0.5f,
                                                          float borderOpacity = -1.0f,
                                                          int borderRadius = 1
          );
    void setTextLabels(
            LabelNames labelNames,
            LabelColors labelColors = LabelColors(),
            float areaThreshold = 1.0f);
protected:
    void createLayout();

    std::map<PlaneType, std::vector<SegmentationRenderer::pointer>> m_segmentationRenderers;
    ImageRenderer::pointer m_imageRenderers[3];
    SegmentationLabelRenderer::pointer m_labelRenderers[3];
    std::map<PlaneType, std::vector<ImageSlicer::pointer>> m_slicers;
    QSlider* m_sliderWidgets[3];
    QSlider* m_levelSlider;
    QSlider* m_windowSlider;
    PlaneType m_slicePlanes[3] = {PLANE_X, PLANE_Y, PLANE_Z};
};

}