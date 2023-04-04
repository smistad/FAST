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
        /**
         * @brief Create instance
         * @param bgcolor Background color
         * @param width Window width
         * @param height Window height
         */
        FAST_CONSTRUCTOR(SlicerWindow,
                         Color, bgcolor, = Color::Black(),
                         uint, width, = 0,
                         uint, height, = 0
        );
        /**
         * @brief Connect an 3D image source to this window
         * @param processObject Process object producing a 3D image
         * @param level Intensity level
         * @param window Intensity window
         * @param outputPortID Output port id of process object
         */
        std::shared_ptr<SlicerWindow> connectImage(std::shared_ptr<ProcessObject> processObject, float level = -1, float window = -1, uint outputPortID = 0);
        /**
         * @brief Connect a 3D image to this window
         * @param image 3D image to connect
         * @param level Intensity level
         * @param window Intensity window
         */
        std::shared_ptr<SlicerWindow> connectImage(std::shared_ptr<Image> image, float level = -1, float window = -1);
        /**
         * @brief Connect a 3D segmentation soruce to this window
         * @param processObject Process object producing a 3D segmentation image
         * @param colors Label colors
         * @param opacity Segmentation overlay opacity
         * @param borderOpacity Segmentation overlay border opacity
         * @param borderRadius How thick, in pixels, the border radius should be
         * @param outputPortID Output port id of process object
         */
        std::shared_ptr<SlicerWindow> connectSegmentation(std::shared_ptr<ProcessObject> processObject,
                                                          LabelColors colors = LabelColors(),
                                                          float opacity = 0.5f,
                                                          float borderOpacity = -1.0f,
                                                          int borderRadius = 1,
                                                          uint outputPortID = 0
          );
        /**
          * @brief Connect a 3D segmentation to this window
          * @param image 3D segmentation image
          * @param colors Label colors
          * @param opacity Segmentation overlay opacity
          * @param borderOpacity Segmentation overlay border opacity
          * @param borderRadius How thick, in pixels, the border radius should be
          */
        std::shared_ptr<SlicerWindow> connectSegmentation(std::shared_ptr<Image> image,
                                                          LabelColors colors = LabelColors(),
                                                          float opacity = 0.5f,
                                                          float borderOpacity = -1.0f,
                                                          int borderRadius = 1
                                                          );
    void setTextLabels(
            LabelNames labelNames,
            LabelColors labelColors = LabelColors(),
            float areaThreshold = 1.0f);

    std::shared_ptr<Window> connect(uint id, std::shared_ptr<DataObject> data) override;
    std::shared_ptr<Window> connect(uint id, std::shared_ptr<ProcessObject> PO, uint portID = 0) override;
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