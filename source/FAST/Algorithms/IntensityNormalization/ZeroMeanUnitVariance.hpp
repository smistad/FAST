#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT ZeroMeanUnitVariance : public ProcessObject {
	FAST_PROCESS_OBJECT(ZeroMeanUnitVariance)
	public:
		FAST_CONSTRUCTOR(ZeroMeanUnitVariance)
	private:
		void execute() override;
};
  
}
