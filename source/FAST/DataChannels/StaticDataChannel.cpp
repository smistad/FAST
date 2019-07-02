#include "StaticDataChannel.hpp"

namespace fast {

DataObject::pointer StaticDataChannel::getNextDataFrame() {
    std::unique_lock<std::mutex> lock(m_mutex);

    // Block until we get any data or a stop signal
    while(getSize() == 0 && !m_stop) {
        m_frameConditionVariable.wait(lock);
    }

    // If stop is signaled, throw an exception to stop the entire computation thread
    if(m_stop)
        throw ThreadStopped();

    DataObject::pointer data = m_frame;

    // For static channels the data is not removed

    return data;
}

}