#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Streamers/Streamer.hpp>
#include <thread>

namespace fast {

class FAST_EXPORT PatchGenerator : public Streamer {
    FAST_OBJECT(PatchGenerator)
    public:
        bool hasReachedEnd() override;
        void setPatchSize(int width, int height);
        void setPatchLevel(int level);
        ~PatchGenerator();
        void stop();
    protected:
        int m_width, m_height;

        bool m_firstFrameIsInserted;
        bool m_streamIsStarted;
        bool m_stop;
        bool m_hasReachedEnd;
        std::mutex m_firstFrameMutex;
        std::mutex m_stopMutex;
        std::unique_ptr<std::thread> m_thread;
        std::condition_variable m_firstFrameCondition;
        SharedPointer<WholeSlideImage> m_inputImage;
        int m_level;

        void execute();
        void generateStream();
    private:
        PatchGenerator();
};
}