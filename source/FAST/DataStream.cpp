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
    if(m_outputPorts.empty())
        throw Exception("Process objects given to DataStream had no output ports");
}

std::shared_ptr<DataObject> DataStream::getNextDataObject(uint portID) {
    if(m_outputPorts.size() <= portID)
        throw Exception("Output nr " + std::to_string(portID) + " does not exist in DataStream.");
    if(m_nextDataObjects.count(portID) == 0) {
        // Run all POs
        for(int i = 0; i < m_outputPorts.size(); ++i) {
            m_outputPorts[i]->getProcessObject()->run(m_executeToken);
            m_nextDataObjects[i] = m_outputPorts[i]->getNextFrame();
            if(m_nextDataObjects[i]->isLastFrame()) {
                m_done = true;
            }
        }
        ++m_executeToken;
    }
    auto result = m_nextDataObjects[portID];
    m_nextDataObjects.erase(portID);
    return std::shared_ptr<DataObject>(result);
}

DataStream::DataStream(std::shared_ptr<ProcessObject> po) : DataStream(std::vector<std::shared_ptr<ProcessObject>>({po})) {

}

}