%include "FAST/ProcessObject.i"
%include "FAST/Visualization/Plane.i"
%shared_ptr(fast::ImageSlicer)

namespace fast {

enum PlaneType {PLANE_X, PLANE_Y, PLANE_Z};

class ImageSlicer : public ProcessObject {
    public:
    	static SharedPointer<ImageSlicer> New();
		void setOrthogonalSlicePlane(PlaneType orthogonalSlicePlane, int sliceNr = -1);
		void setArbitrarySlicePlane(Plane slicePlane);
	private:
		ImageSlicer();
};

// This must come after class declaration
%template(ImageSlicerPtr) SharedPointer<ImageSlicer>;
}
