#include "DataChannel.hpp"

namespace fast {
void DataChannel::stop() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }
}

DataChannel::DataChannel() {
    m_stop = false;
}

SharedPointer<ProcessObject> DataChannel::getProcessObject() const {
    return m_processObject;
}

void DataChannel::setProcessObject(SharedPointer<ProcessObject> po) {
    m_processObject = po;
}

template <>
SharedPointer<DataObject> DataChannel::getNextFrame<DataObject>() {
    return getNextDataFrame();
}

}