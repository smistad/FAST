#ifndef AIRWAY_SEGMENTATION_HPP_
#define AIRWAY_SEGMENTATION_HPP_

#include "FAST/Algorithms/SegmentationAlgorithm.hpp"

namespace fast {

class Image;
class Segmentation;

class FAST_EXPORT  AirwaySegmentation : public SegmentationAlgorithm {
	FAST_OBJECT(AirwaySegmentation)
	public:
	private:
		AirwaySegmentation();
		void execute();
		static Vector3i findSeedVoxel(SharedPointer<Image> volume);

		SharedPointer<Image> convertToHU(SharedPointer<Image> image);
		void morphologicalClosing(SharedPointer<Segmentation> segmentation);
};

}


#endif
