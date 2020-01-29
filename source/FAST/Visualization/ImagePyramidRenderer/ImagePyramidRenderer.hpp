#pragma once

#include <FAST/Visualization/Renderer.hpp>
#include <deque>
#include <thread>

namespace fast {

class ImagePyramid;

class FAST_EXPORT ImagePyramidRenderer : public Renderer {
    FAST_OBJECT(ImagePyramidRenderer)
    public:
        void loadAttributes() override;
        void setIntensityLevel(float level);
        float getIntensityLevel();
        void setIntensityWindow(float window);
        float getIntensityWindow();
        ~ImagePyramidRenderer() override;
        void clearPyramid();
    private:
        ImagePyramidRenderer();
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

        // Level and window intensities
        float mWindow;
        float mLevel;

        void drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D);
};

}