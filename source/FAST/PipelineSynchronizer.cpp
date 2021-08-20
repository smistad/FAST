#include "PipelineSynchronizer.hpp"

namespace fast {

PipelineSynchronizer::PipelineSynchronizer() {

}

uint PipelineSynchronizer::addInputConnection(DataChannel::pointer channel) {
    uint nr = getNrOfInputConnections();
    createInputPort<DataObject>(nr);
    createOutputPort<DataObject>(nr);
    setInputConnection(nr, channel);
    return nr;
}

void PipelineSynchronizer::execute() {
    // Every time a new data object arrives on one of the input connections:
    // Send out all recent data objects
    const int size = getNrOfInputConnections();
    if(size < 2)
        throw Exception("More than one connection has to provided to PipelineSynchronizer");

    // Update latestData with any new data that has arrived
    for(int portID = 0; portID < size; ++portID) {
        if(mInputConnections[portID]->hasCurrentData()) {
            m_latestData[portID] = mInputConnections[portID]->getNextFrame();
        }
    }

    // If all connections have data; send it foward
    if(m_latestData.size() == size) {
        for(int portID = 0; portID < size; ++portID) {
            addOutputData(portID, m_latestData[portID]);
        }
    }
}

}