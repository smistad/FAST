#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Normalize intensities of an image to have zero mean and unit variance
 *
 * This process object will scale the pixel values so that the resulting image
 * has a zero mean and unit variance.
 * This achieved by doing (image - mean(image)) / std(image)
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs:
 * - 0: Image float
 *
 * @ingroup filter
 */
class FAST_EXPORT ZeroMeanUnitVariance : public ProcessObject {
	FAST_PROCESS_OBJECT(ZeroMeanUnitVariance)
	public:
        /**
         * @brief Create instance
         * @return instance
         */
		FAST_CONSTRUCTOR(ZeroMeanUnitVariance)
	private:
		void execute() override;
};
  
}
