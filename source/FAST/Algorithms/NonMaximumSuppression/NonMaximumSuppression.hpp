#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Non-maximum suppression of bounding box sets
 *
 * Removes overlapping bounding boxes in a BoundingBoxSet if intersection over union is above a provided threshold.
 *
 * Inputs:
 * - 0: BoundingBoxSet
 *
 * Outputs:
 * - 0: BoundingBoxSet
 *
 * @ingroup bounding-box
 */
class FAST_EXPORT NonMaximumSuppression : public ProcessObject {
	FAST_PROCESS_OBJECT(NonMaximumSuppression)
	public:
        /**
         * @brief Create instance
         * @param threshold Minimum intersection over union to remove overlapping bounding box.
         * @return instance
         */
        FAST_CONSTRUCTOR(NonMaximumSuppression, float, threshold, = 0.5f);
		void setThreshold(float threshold);
		void loadAttributes();
	protected:
		void execute() override;

		float m_threshold = 0.5f;
};

}