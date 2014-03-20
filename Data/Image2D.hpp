#ifndef IMAGE2D_HPP
#define IMAGE2D_HPP

#include "ImageData.hpp"
namespace fast {

class Image2D : public ImageData {
    private:
        void execute();
};

typedef boost::shared_ptr<Image2D> Image2DPtr;

}; // end namespace fast

#endif
