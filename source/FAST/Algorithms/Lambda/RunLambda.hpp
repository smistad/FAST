#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT RunLambda : public ProcessObject {
	FAST_OBJECT(RunLambda)
	public:
		void setLambda(std::function<void()> lambda);
		void setLambda(std::function<void(DataObject::pointer)> lambda);
		void setRunOnLastFrameOnly(bool lastFrameOnly);
	protected:
		RunLambda();
		void execute();

		std::function<void()> m_lambda;
		std::function<void(DataObject::pointer)> m_lambdaWithInput;
		bool m_runOnLastFrameOnly = false;
};

class FAST_EXPORT RunLambdaOnLastFrame : public RunLambda {
	FAST_OBJECT(RunLambdaOnLastFrame);
	void setRunOnLastFrameOnly(bool) = delete;
	protected:
		RunLambdaOnLastFrame();

};

}