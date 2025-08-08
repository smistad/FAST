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
    std::lock_guard<std::mutex> lock(m_stopMutex);
    if(!m_thread) // Already stopped
        return;
    if(std::this_thread::get_id() == m_thread->get_id())
        throw Exception("You can't call Streamer::stop() from the streaming thread.");
    reportInfo() << "Stopping in streamer.." << reportEnd();
    m_stop = true;
    // Thread might be locked due th QueueDataChannel, we have to signal them to stop:
    for(const auto& output : mOutputConnections) {
        for(const auto& channel : output.second) {
            auto channelLocked = channel.lock();
            if(channelLocked) { // Make sure it is still valid, before calling stop
                channelLocked->stop();
            }
        }
    }
    frameAdded(); // Unblock in execute, if needed
    // Join thread and reset
    m_thread->join();
    m_thread = nullptr;
    m_streamIsStarted = false;
    m_firstFrameIsInserted = false;
    reportInfo() << "Streamer::stop() done." << reportEnd();
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
    if(m_outputPOs.count(portID) == 0) { // Doesn't exist
        auto channel = ProcessObject::getOutputPort(portID);
        auto PO = RunLambda::create([](DataObject::pointer data) -> DataList {
            return DataList(data);
        });
        PO->setInputConnection(channel);
        m_outputPOs[portID] = PO;
        return PO->getOutputPort();
    } else {
        auto PO = m_outputPOs[portID].lock();
        if(!PO) { // Expired, recreate
            // TODO duplicate code:
            auto channel = ProcessObject::getOutputPort(portID);
            auto PO = RunLambda::create([](DataObject::pointer data) -> DataList {
                return DataList(data);
            });
            PO->setInputConnection(channel);
            m_outputPOs[portID] = PO;
        }

        return PO->getOutputPort();
    }
}

bool Streamer::isStopped() {
    return m_stop;
}

Streamer::~Streamer() {
    reportInfo() << "Destroying streamer.." << reportEnd();
    stop();
    reportInfo() << "Streamer DESTROYED." << reportEnd();
}

void Streamer::stopWithError(std::string message, int outputPort) {
    frameAdded(); // Unlock blocking execute()
    if(outputPort < 0)
        outputPort = 0;
    for(const auto& channel : mOutputConnections[outputPort]) {
        auto channelLocked = channel.lock();
        if(channelLocked) // Make sure it is still valid, before calling stop
            channelLocked->stop();
    }
}

}
