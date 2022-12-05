#include "NewestFrameDataChannel.hpp"

namespace fast {

void NewestFrameDataChannel::addFrame(DataObject::pointer data) {
    // Simply replace any previous data
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_frame = data;
    }
    m_frameConditionVariable.notify_one();
}

DataObject::pointer NewestFrameDataChannel::getNextDataFrame() {
    std::unique_lock<std::mutex> lock(m_mutex);

    // Block until we get any data or a stop signal
    while(getSize() == 0 && !m_stop) {
        m_frameConditionVariable.wait(lock);
    }

    // If stop is signaled, throw an exception to stop the entire computation thread
    if(m_stop)
        throw ThreadStopped(m_errorMessage);

    DataObject::pointer data = m_frame;

    // Remove frame as we don't want to process the same frame again
    m_frame.reset();

    return data;
}

int NewestFrameDataChannel::getSize() {
    return m_frame ? 1 : 0;
}

void NewestFrameDataChannel::setMaximumNumberOfFrames(uint frames) {

}

int NewestFrameDataChannel::getMaximumNumberOfFrames() const {
    return 1;
}

void NewestFrameDataChannel::stop(std::string errorMessage) {
    DataChannel::stop(errorMessage);

    // Since getNextFrame might be waiting for data, we need to notify it to stop blocking
    Reporter::info() << "Notifying condition variable" << Reporter::end();
    m_frameConditionVariable.notify_all();
}

bool NewestFrameDataChannel::hasCurrentData() {
    return m_frame ? true: false;
}

DataObject::pointer NewestFrameDataChannel::getFrame() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(getSize() == 0)
        throw Exception("No frames available in getFrame");
    return m_frame;
}


}