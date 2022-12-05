#pragma once

#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Color.hpp"
#include "FAST/Utility.hpp"
#include <unordered_map>
#include <mutex>
#include <FAST/Visualization/LabelColorRenderer.hpp>

namespace fast {

class ImagePyramid;

/**
 * @brief Renders 2D segmentation data
 *
 * Renders segmentation data using colors and potentially transparency.
 *
 * Input can be 2D Segmentation, Image or ImagePyramid (of type TYPE_UINT8) objects.
 *
 * @ingroup renderers
 */
class FAST_EXPORT  SegmentationRenderer : public ImageRenderer, public LabelColorRenderer {
    FAST_PROCESS_OBJECT(SegmentationRenderer)
    public:
        FAST_CONSTRUCTOR(SegmentationRenderer,
                         LabelColors, colors, = LabelColors(),
                         float, opacity, = 0.5f,
                         float, borderOpacity, = -1.0f,
                         int, borderRadius, = 1
        )
        void setBorderRadius(int radius);
        void setOpacity(float opacity, float borderOpacity = -1);
        void setBorderOpacity(float borderOpacity);
        float getOpacity() const;
        float getBorderOpacity() const;
        int getBorderRadius() const;
        std::string attributesToString() override;
        void loadAttributes() override;
        virtual ~SegmentationRenderer();
    protected:
        void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight) override;
        void drawPyramid(std::shared_ptr<SpatialDataObject> dataToRender, Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar);
        void drawNormal(std::unordered_map<uint, std::shared_ptr<SpatialDataObject>> dataToRender, Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) ;
        virtual void deleteAllTextures() override;

        int mBorderRadius = 1;
        float mBorderOpacity = 0.5;

        // Queue of tiles to be loaded
        std::list<std::string> m_tileQueue; // LIFO queue of unique items
        // Buffer to process queue
        std::unique_ptr<std::thread> m_bufferThread;
        // Condition variable to wait if queue is empty
        std::condition_variable m_queueEmptyCondition;
        std::mutex m_tileQueueMutex;
        bool m_stop = false;
        std::unordered_set<std::string> m_loaded;

        int m_currentLevel = -1;

        std::shared_ptr<ImagePyramid> m_input;

        std::atomic<uint64_t> m_memoryUsage;
        std::mutex m_texturesToRenderMutex;
        std::unordered_map<std::string, uint> mPyramidTexturesToRender;
        std::unordered_map<std::string, uint> mPyramidVAO;
        std::unordered_map<std::string, uint> mPyramidVBO;
        std::unordered_map<std::string, uint> mPyramidEBO;
};

} // end namespace fast

