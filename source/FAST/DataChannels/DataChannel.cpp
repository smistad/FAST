#include "DataChannel.hpp"

namespace fast {
void DataChannel::stop(std::string errorMessage) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
        m_errorMessage = errorMessage;
    }
}

DataChannel::DataChannel() {
    m_stop = false;
}

std::shared_ptr<ProcessObject> DataChannel::getProcessObject() const {
    return m_processObject;
}

void DataChannel::setProcessObject(std::shared_ptr<ProcessObject> po) {
    m_processObject = po;
}

template <>
std::shared_ptr<DataObject> DataChannel::getNextFrame<DataObject>() {
    return getNextDataFrame();
}

}