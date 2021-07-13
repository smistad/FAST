#include "RunLambda.hpp"

#include <utility>

namespace fast {

RunLambda::RunLambda() {
    for(int i = 0; i < 10;++i) {
        // TODO disable input/output port exists instead..
        createInputPort(i, "", "", false);
        createOutputPort(i, "", "");
    }
}

RunLambda::RunLambda(std::function<DataList()> lambda) : RunLambda() {
    m_lambdaNoInput = std::move(lambda);
    setModified(true);
}

RunLambda::RunLambda(std::function<DataList(DataObject::pointer)> lambda) : RunLambda() {
    m_lambdaWithSingleInput = std::move(lambda);
    setModified(true);
}

RunLambda::RunLambda(std::function<DataList(DataList)> lambda) : RunLambda() {
    m_lambdaWithMultipleInput = std::move(lambda);
    setModified(true);
}

void RunLambda::setRunOnLastFrameOnly(bool lastFrameOnly) {
	m_runOnLastFrameOnly = lastFrameOnly;
	setModified(true);
}

void RunLambda::execute() {
    // Collect input data to DataList
    std::map<int, DataObject::pointer> map;
    bool isLastFrame = false;
    for(auto inputConnection : mInputConnections) {
        map[inputConnection.first] = getInputData<DataObject>(inputConnection.first);
        if(map[inputConnection.first]->isLastFrame())
            isLastFrame = true;
    }
    DataList dataList(map);
    DataList output;
	if(!m_runOnLastFrameOnly || isLastFrame) {
		if(m_lambdaNoInput) {
			output = m_lambdaNoInput();
		} else if(m_lambdaWithSingleInput) {
            output = m_lambdaWithSingleInput(map.begin()->second);
        } else if(m_lambdaWithMultipleInput) {
		    output = m_lambdaWithMultipleInput(dataList);
		} else {
			throw Exception("No lambda was given to RunLambda");
		}
	}
	for(auto data : output.getAllData())
        addOutputData(data.first, data.second);
}

}