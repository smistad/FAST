#include "RunLambda.hpp"

namespace fast {

RunLambda::RunLambda() {
	createInputPort<DataObject>(0);
	createOutputPort<DataObject>(0);
}

void RunLambda::setLambda(std::function<void()> lambda) {
	m_lambda = lambda;
	setModified(true);
}

void RunLambda::setLambda(std::function<void(DataObject::pointer)> lambda) {
	m_lambdaWithInput = lambda;
	setModified(true);
}

void RunLambda::setRunOnLastFrameOnly(bool lastFrameOnly) {
	m_runOnLastFrameOnly = lastFrameOnly;
	setModified(true);
}

void RunLambda::execute() {
	auto input = getInputData<DataObject>();
	if(!m_runOnLastFrameOnly || input->isLastFrame()) {
		if(m_lambda) {
			m_lambda();
		} else if(m_lambdaWithInput) {
			m_lambdaWithInput(input);
		} else {
			throw Exception("No lambda was given to RunLambda");
		}
	}
	addOutputData(0, input);
}

RunLambdaOnLastFrame::RunLambdaOnLastFrame() {
	m_runOnLastFrameOnly = true;
}

}