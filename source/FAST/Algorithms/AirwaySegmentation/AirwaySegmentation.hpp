#ifndef AIRWAY_SEGMENTATION_HPP_
#define AIRWAY_SEGMENTATION_HPP_

#include "FAST/Algorithms/SegmentationAlgorithm.hpp"

namespace fast {

class AirwaySegmentation : public SegmentationAlgorithm {
	FAST_OBJECT(AirwaySegmentation)
	public:
	private:
		AirwaySegmentation();
		void execute();
};

}


#endif
