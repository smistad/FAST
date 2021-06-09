#include "DataStream.hpp"
#include "ProcessObject.hpp"

namespace fast {

std::vector<std::shared_ptr<DataChannel>> _DataStreamGetOutputPorts(std::shared_ptr<ProcessObject> po) {
    std::vector<std::shared_ptr<DataChannel>> outputPorts;
    for(int i = 0; i < po->getNrOfOutputPorts(); ++i) {
        outputPorts.push_back(po->getOutputPort(i));
    }
    return outputPorts;
}

void _DataStreamRunPipeline(std::shared_ptr<ProcessObject> po) {
    po->run();
}

}