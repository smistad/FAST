#ifndef IMAGE2D_HPP
#define IMAGE2D_HPP

#include "ImageData.hpp"
#include "OpenCLManager.hpp"
namespace fast {

class Image2D: public ImageData {
    public:
        typedef boost::shared_ptr<Image2D> Ptr;
        static Image2D::Ptr New() {
            Image2D * ptr = new Image2D();
            Image2D::Ptr smartPtr(ptr);
            ptr->setPtr(smartPtr);

            return smartPtr;
        }
        void addParent(PipelineObject::Ptr parent);
        void setOpenCLImage(cl::Image2D clImage, oul::Context context);
    private:
        Image2D() {};
        void setPtr(Image2D::Ptr ptr) {mPtr = ptr;};
        Image2D::Ptr mPtr;
        // These two vectors should be equal in size and have entries
        // that correspond to eachother
        std::vector<cl::Image2D> mCLImages;
        std::vector<oul::Context> mCLContexes;
        void execute(){};
};

typedef boost::shared_ptr<Image2D> Image2DPtr;

}
;
// end namespace fast

#endif
