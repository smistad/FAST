#include "DataStream.hpp"
#include "ProcessObject.hpp"

namespace fast {

bool DataStream::isDone() {
    return m_done;
}

DataStream::DataStream(std::vector<std::shared_ptr<ProcessObject>> processObjects) {
    m_processObjects = processObjects;
    for(auto po : m_processObjects) {
        for(int i = 0; i < po->getNrOfOutputPorts(); ++i) {
            m_outputPorts.push_back(po->getOutputPort(i));
        }
    }
    if(m_outputPorts.size() == 0)
        throw Exception("Process objects given to DataStream had no output ports");
}

std::shared_ptr<DataObject> DataStream::getNextDataObject(uint portID) {
    if(m_outputPorts.size() >= portID)
        throw Exception("Output nr " + std::to_string(portID) + " does not exist.");
    if(m_nextDataObjects.count(portID) == 0) {
        // Missing data, have to run..
        // TODO what to do if there is one output port forgotten, and it fills up to the point it will block..
        // TODO Detect and warn?
        m_outputPorts[portID]->getProcessObject()->run(m_executeToken);
        ++m_executeToken;
        for(int i = 0; i < m_outputPorts.size(); ++i) {
            m_nextDataObjects[i] = m_outputPorts[i]->getNextFrame();
            if(m_nextDataObjects[i]->isLastFrame())
                m_done = true;
        }
    }
    auto result = m_nextDataObjects[portID];
    m_nextDataObjects.erase(portID);
    return std::shared_ptr<DataObject>();
}

DataStream::DataStream(std::shared_ptr<ProcessObject> po) : DataStream(std::vector<std::shared_ptr<ProcessObject>>({po})) {

}

}