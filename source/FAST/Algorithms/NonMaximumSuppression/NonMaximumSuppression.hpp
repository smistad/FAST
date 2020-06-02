#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class NonMaximumSuppression : public ProcessObject {
	FAST_OBJECT(NonMaximumSuppression)
	public:
		void setThreshold(float threshold);
		void loadAttributes();
	protected:
		NonMaximumSuppression();
		void execute() override;

		float m_threshold = 0.5f;
};

}