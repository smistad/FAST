#include "Streamer.hpp"

namespace fast {

void Streamer::waitForFirstFrame() {
    // Wait here for first frame
    std::unique_lock<std::mutex> lock(m_firstFrameMutex);
    while(!m_firstFrameIsInserted) {
        m_firstFrameCondition.wait(lock);
    }
}

void Streamer::startStream() {
    if(!m_streamIsStarted) {
        m_streamIsStarted = true;
        m_thread = std::make_unique<std::thread>(std::bind(&Streamer::generateStream, this));
    }
}

void Streamer::frameAdded() {
    {
        std::unique_lock<std::mutex> lock(m_firstFrameMutex);
        m_firstFrameIsInserted = true;
    }
    m_firstFrameCondition.notify_one();
}

Streamer::Streamer() {
    m_firstFrameIsInserted = false;
    m_streamIsStarted = false;
    m_stop = false;
}

void Streamer::stop() {
    {
        std::unique_lock<std::mutex> lock(m_stopMutex);
        m_stop = true;
    }
    if(m_thread) {
        m_thread->join();
        m_thread = NULL;
        reportInfo() << "Streamer thread for " << getNameOfClass() << " returned" << reportEnd();
    }
}

}
