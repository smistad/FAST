#pragma once

#include <FAST/Visualization/Renderer.hpp>
#include <deque>
#include <thread>
#include <FAST/Data/Color.hpp>

namespace fast {

class ImagePyramid;

class FAST_EXPORT SegmentationPyramidRenderer : public Renderer {
    FAST_OBJECT(SegmentationPyramidRenderer)
    public:
        void loadAttributes() override;
        ~SegmentationPyramidRenderer() override;
        void clearPyramid();
        void setOpacity(float opacity);
    private:
        SegmentationPyramidRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);

        std::unordered_map<std::string, uint> mTexturesToRender;
        std::unordered_map<uint, SharedPointer<ImagePyramid>> mImageUsed;
        std::unordered_map<std::string, uint> mVAO;
        std::unordered_map<std::string, uint> mVBO;
        std::unordered_map<std::string, uint> mEBO;

        // Queue of tiles to be loaded
        std::deque<std::string> m_tileQueue; // LIFO queue
        // Buffer to process queue
        std::unique_ptr<std::thread> m_bufferThread;
        // Condition variable to wait if queue is empty
        std::condition_variable m_queueEmptyCondition;
        std::mutex m_tileQueueMutex;
        bool m_stop = false;
        std::unordered_set<std::string> m_loaded;

        int m_currentLevel;

        cl::Kernel mKernel;

        SharedPointer<ImagePyramid> m_input;

		bool mColorsModified;
        bool mFillAreaModified;

        std::unordered_map<int, Color> mLabelColors;
        std::unordered_map<int, bool> mLabelFillArea;
        bool mFillArea;
        int mBorderRadius = 1;
        float mOpacity = 0.5;
        cl::Buffer mColorBuffer, mFillAreaBuffer;
        std::atomic<uint64_t> m_memoryUsage;
        std::mutex m_texturesToRenderMutex;

        void drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D);
};

}