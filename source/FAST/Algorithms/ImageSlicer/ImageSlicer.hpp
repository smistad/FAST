#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Visualization/Plane.hpp"
#include "FAST/Data/DataTypes.hpp"

namespace fast {

class Image;

/**
 * @brief Slice a 3D image using a defined plane
 *
 * @sa Plane PlaneType
 */
class FAST_EXPORT ImageSlicer : public ProcessObject {
	FAST_PROCESS_OBJECT(ImageSlicer)
	public:
        FAST_CONSTRUCTOR(ImageSlicer, Plane, slicePlane,);
        FAST_CONSTRUCTOR(ImageSlicer, PlaneType, orthogoalSlicePlane,, int, sliceNr, = -1);
        void setOrthogonalSlicePlane(PlaneType orthogonalSlicePlane, int sliceNr = -1);
		void setArbitrarySlicePlane(Plane slicePlane);
	private:
        void init();
		ImageSlicer();
		void execute();
		std::shared_ptr<Image> orthogonalSlicing(std::shared_ptr<Image> input);
		std::shared_ptr<Image> arbitrarySlicing(std::shared_ptr<Image> input);

		bool mOrthogonalSlicing;
		bool mArbitrarySlicing;
		Plane mArbitrarySlicePlane;
		PlaneType mOrthogonalSlicePlane;
		int mOrthogonalSliceNr;
};

} // end namespace fast
