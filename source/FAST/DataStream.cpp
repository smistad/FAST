#include "DataStream.hpp"
#include "ProcessObject.hpp"

namespace fast {

DataStream::DataStream(std::shared_ptr<ProcessObject> po) {
    m_processObject = po;
    for(int i = 0; i < po->getNrOfOutputPorts(); ++i) {
        m_outputPorts.push_back(po->getOutputPort(i));
    }
}

template<class DataType>
void DataStream::getNextFrame(uint outputPortID) {
    if(m_outputPorts.size() >= outputPortID)
        throw Exception("Output port " + std::to_string(outputPortID) + " does not exist.");
    if(m_lastDataObject.count(outputPortID) == 0) {
        // Missing data, have to run..
        // TODO what to do if there is one output port forgotten, and it fills up to the point it will block..
        // TODO Detect and warn?
        m_processObject->run();
        for(int i = 0; i < m_processObject->getNrOfOutputPorts(); ++i) {
            m_lastDataObject[i] = m_outputPorts[outputPortID]->getNextFrame();
        }
    }
    auto result = m_lastDataObject[outputPortID];
    m_lastDataObject.erase(outputPortID);
    return std::dynamic_pointer_cast<DataType>(result);
}

}