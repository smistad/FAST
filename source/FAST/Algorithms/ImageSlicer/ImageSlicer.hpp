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
        /**
        * @brief Create instance
        *
        * Slice a 3D image with an orthogonal slice plane. Default slice nr is the center slice.
        *
        * @param orthogonalSlicePlane Which orthogonal slice plane to use X/Y/Z
        * @param sliceNr Which slice nr to extract, must be smaller than size of the slicing dimension.
        *      If negative center slice will be used
        * @return instance
        */
        FAST_CONSTRUCTOR(ImageSlicer, PlaneType, orthogoalSlicePlane,, int, sliceNr, = -1);
        /**
         * @brief Create instance
         *
         * Slice a volume with an arbitrary slice plane defined by Plane.
         *
         * @param arbitrarySlicePlane
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageSlicer, Plane, slicePlane,);
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
