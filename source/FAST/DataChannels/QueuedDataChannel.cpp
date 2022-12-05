#include "QueuedDataChannel.hpp"

namespace fast {

void QueuedDataChannel::addFrame(DataObject::pointer data) {

    //if(!mGetCalled && mFillCount->getCount() == mMaximumNumberOfFrames)
    //    Reporter::error() << "EXECUTION BLOCKED by DataChannel from " << mProcessObject->getNameOfClass() << ". Do you have a DataChannel object that is not used?" << Reporter::end();

    // Decrement semaphore by one, wait if queue is full
    m_emptyCount->wait();

    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // If stop is signaled, throw an exception to stop the entire computation thread
        if(m_stop)
            throw ThreadStopped(m_errorMessage);

        m_queue.push(data);
    }

    // Increment semaphore by one, signal any waiting due to empty queue
    m_fillCount->signal();
}

DataObject::pointer QueuedDataChannel::getNextDataFrame() {
    // Decrement semaphore by one, and wait if queue is empty
    m_fillCount->wait();

    DataObject::pointer data;
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // If stop is signaled, throw an exception to stop the entire computation thread
        if(m_stop)
            throw ThreadStopped(m_errorMessage);

        // Get frame next in queue and remove it from the queue
        data = m_queue.front();
        m_queue.pop();
    }

    // Increment semaphore by one and signal any waiting for next frame due to empty queue
    m_emptyCount->signal();

    return data;
}

int QueuedDataChannel::getSize() {
    return m_queue.size();
}

void QueuedDataChannel::setMaximumNumberOfFrames(uint frames) {
    if(!m_queue.empty())
        throw Exception("Have to call setMaximumNumberOfFrames before executing pipeline");
    mMaximumNumberOfFrames = frames;
    m_fillCount = std::make_unique<LightweightSemaphore>(0);
    m_emptyCount = std::make_unique<LightweightSemaphore>(mMaximumNumberOfFrames);
}

int QueuedDataChannel::getMaximumNumberOfFrames() const {
    return mMaximumNumberOfFrames;
}

void QueuedDataChannel::stop(std::string errorMessage) {
    DataChannel::stop(errorMessage);
    Reporter::info() << "SIGNALING SEMAPHORES in QueuedDataChannel" << Reporter::end();

    // Since getNextFrame or addFrame might be waiting for data, we need to signal the semaphore to stop them blocking
    m_fillCount->signal();
    m_emptyCount->signal();
}

bool QueuedDataChannel::hasCurrentData() {
    return getSize() > 0;
}

DataObject::pointer QueuedDataChannel::getFrame() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_queue.empty())
        throw Exception("No frames available in getFrame");
    return m_queue.front();
}

QueuedDataChannel::QueuedDataChannel() {
    setMaximumNumberOfFrames(50);
}

}
