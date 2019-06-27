#pragma once

#include <FAST/Streamers/Streamer.hpp>
#include <thread>

namespace fast {

class Image;

class FAST_EXPORT ImageToBatchGenerator : public Streamer {
    FAST_OBJECT(ImageToBatchGenerator)
    public:
        void setMaxBatchSize(int size);
        bool hasReachedEnd() override;
        ~ImageToBatchGenerator() override;
    protected:
        void execute() override;
        void generateStream();
        int m_maxBatchSize;

        bool m_firstFrameIsInserted;
        bool m_streamIsStarted;
        bool m_stop;
        bool m_hasReachedEnd;
        std::mutex m_firstFrameMutex;
        std::mutex m_stopMutex;
        std::unique_ptr<std::thread> m_thread;
        std::condition_variable m_firstFrameCondition;
    private:
        ImageToBatchGenerator();
};

}