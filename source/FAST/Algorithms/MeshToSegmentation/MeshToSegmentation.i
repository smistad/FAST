%include "FAST/ProcessObject.i"
%shared_ptr(fast::SegmentationAlgorithm)
%shared_ptr(fast::MeshToSegmentation)

namespace fast {

%ignore SegmentationAlgorithm;
class SegmentationAlgorithm : public ProcessObject {

};
%template(SegmentationAlgorithmPtr) std::shared_ptr<SegmentationAlgorithm>;

class MeshToSegmentation : public SegmentationAlgorithm {
    public:
    	static std::shared_ptr<MeshToSegmentation> New();
        /**
         * Set output image resolution in voxels
         * @param x
         * @param y
         * @param z
         */
		void setOutputImageResolution(uint x, uint y, uint z = 1);
	private:
		MeshToSegmentation();
};

// This must come after class declaration
%template(MeshToSegmentationPtr) std::shared_ptr<MeshToSegmentation>;
}
