#pragma once

#include <FAST/Visualization/Renderer.hpp>
#include <deque>
#include <thread>
#include <FAST/Data/Color.hpp>

namespace fast {

class ImagePyramid;

/*
template <class T>
class UniqueQueue {
    public:
        T next_and_pop() {
            std::lock_guard<std::mutex> lock(m_mutex);
            T item = m_queue.back();
            m_queue.pop_back();
            m_indices.erase(item);
            return item;
        }
        void add(T item) {
            std::lock_guard<std::mutex> lock(m_mutex);
            //std::cout << "Adding item " << item << std::endl;
            if(m_indices.count(item) > 0) {
				// To avoid duplicates
                auto it = m_queue.begin();
                if(m_indices[item] < m_queue.size()) {
                    //std::cout << "Duplicate, erasing.. " << m_indices[item] << " " << m_queue.size() << std::endl;
                    std::advance(it, m_indices[item]);
                    m_queue.erase(it);
                    // Update indices of items after the deleted item
                    for(;it != m_queue.end(); ++it) // TODO find a more efficient way to do this
                        m_indices[*it]--;
                    //std::cout << "New size: " << m_queue.size() << std::endl;
                } else {
                    std::cout << "error.." << std::endl;
                }
            }
			m_queue.push_back(item);
            m_indices[item] = m_queue.size() - 1;
        }
        bool empty() {
            return m_queue.empty();
        }
        std::size_t size() {
            return m_queue.size();
        }
        void clear() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.clear();
            m_indices.clear();
        }
        std::mutex& getMutex() {
            return m_mutex;
        }
    private:
        std::list<T> m_queue;
        std::unordered_map<T, std::size_t> m_indices;
        std::mutex m_mutex;
};*/


class FAST_EXPORT SegmentationPyramidRenderer : public Renderer {
    FAST_OBJECT(SegmentationPyramidRenderer)
    public:
        void loadAttributes() override;
        ~SegmentationPyramidRenderer() override;
        void clearPyramid();
        void setOpacity(float opacity);
        void stopPipeline() override;
    private:
        SegmentationPyramidRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);

        std::unordered_map<std::string, uint> mTexturesToRender;
        std::unordered_map<uint, SharedPointer<ImagePyramid>> mImageUsed;
        std::unordered_map<std::string, uint> mVAO;
        std::unordered_map<std::string, uint> mVBO;
        std::unordered_map<std::string, uint> mEBO;

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