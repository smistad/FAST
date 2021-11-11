#include "RunUntilFinished.hpp"

namespace fast {

RunUntilFinished::RunUntilFinished() {
    createInputPort(0);
    createOutputPort(0);
}

void RunUntilFinished::execute() {
    // TODO this assumes the parent only has 1 output port
    auto parentPO = mInputConnections[0]->getProcessObject();
    DataObject::pointer inputData;
    do {
        inputData = parentPO->runAndGetOutputData();
    } while(!inputData->isLastFrame());
    addOutputData(0, inputData);
}

}