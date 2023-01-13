#include <FAST/Algorithms/Lambda/RunLambda.hpp>
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
    m_frameData["streaming"] = "yes";
}

void Streamer::stop() {
    reportInfo() << "Stopping in streamer.." << reportEnd();
    {
        std::unique_lock<std::mutex> lock(m_stopMutex);
        m_stop = true;
        m_streamIsStarted = false;
        m_firstFrameIsInserted = false;
    }
    if(m_thread) {
        m_thread->join();
        m_thread = nullptr;
        reportInfo() << "Streamer thread for " << getNameOfClass() << " returned" << reportEnd();
    }
}

void Streamer::setMaximumNrOfFrames(int frames) {
    m_maximumNrOfFrames = frames;
}

StreamingMode Streamer::getStreamingMode() const {
    return m_streamingMode;
}

void Streamer::setStreamingMode(StreamingMode mode) {
    m_streamingMode = mode;
}

DataChannel::pointer Streamer::getOutputPort(uint portID) {
    if(m_outputPOs.count(portID) == 0) {
        auto channel = ProcessObject::getOutputPort(portID);
        m_outputPOs[portID] = RunLambda::create([](DataObject::pointer data) -> DataList {
            return DataList(data);
        });
        m_outputPOs[portID]->setInputConnection(channel);
    }

    return m_outputPOs[portID]->getOutputPort();
}

bool Streamer::isStopped() {
    std::lock_guard<std::mutex> lock(m_stopMutex);
    return m_stop;
}

}
