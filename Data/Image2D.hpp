#ifndef IMAGE2D_HPP
#define IMAGE2D_HPP

#include "ImageData.hpp"
#include "OpenCLManager.hpp"
namespace fast {

class Image2D: public ImageData {
    public:
        Image2D(
                PipelineObject * parent,
                oul::Context context,
                cl::Image2D clImage,
                unsigned int width,
                unsigned int height,
                DataType type);
    private:
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
