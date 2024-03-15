#include "RunUntilFinished.hpp"

namespace fast {

RunUntilFinished::RunUntilFinished() {
    createInputPort(0);
    createOutputPort(0);
    m_finished = false;
}

void RunUntilFinished::execute() {
    auto parentPO = mInputConnections[0]->getProcessObject();
    if(m_finished) {
        return;
    }
    // TODO this assumes the parent only has 1 output port
    DataObject::pointer inputData;
    Progress progress(1000);
    progress.setText("Running ");
    do {
        inputData = parentPO->runAndGetOutputData();
        if(inputData->hasFrameData("progress"))
            progress.update(round(1000*inputData->getFrameData<float>("progress")));
    } while(!inputData->isLastFrame());
    progress.update(1000);
    m_finished = true;
    addOutputData(0, inputData);
}

}