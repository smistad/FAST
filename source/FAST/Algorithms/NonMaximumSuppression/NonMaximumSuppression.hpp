#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT NonMaximumSuppression : public ProcessObject {
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