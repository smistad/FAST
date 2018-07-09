#ifndef IMAGE_SLICER_HPP
#define IMAGE_SLICER_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Visualization/Plane.hpp"
#include "FAST/Data/DataTypes.hpp"

namespace fast {

class Image;

class FAST_EXPORT  ImageSlicer : public ProcessObject {
	FAST_OBJECT(ImageSlicer)
	public:
		void setOrthogonalSlicePlane(PlaneType orthogonalSlicePlane, int sliceNr = -1);
		void setArbitrarySlicePlane(Plane slicePlane);
	private:
		ImageSlicer();
		void execute();
		void orthogonalSlicing(std::shared_ptr<Image> input, std::shared_ptr<Image> output);
		void arbitrarySlicing(std::shared_ptr<Image> input, std::shared_ptr<Image> output);

		bool mOrthogonalSlicing;
		bool mArbitrarySlicing;
		Plane mArbitrarySlicePlane;
		PlaneType mOrthogonalSlicePlane;
		int mOrthogonalSliceNr;
};

} // end namespace fast

#endif
