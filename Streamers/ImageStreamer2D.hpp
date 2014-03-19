#ifndef IMAGE_STREAMER_2D_HPP
#define IMAGE_STREAMER_2D_HPP

#include "Streamer.hpp"
#include "Image2Dt.hpp"

namespace fast {

class ImageStreamer2D;
typedef boost::shared_ptr<ImageStreamer2D> ImageStreamer2DPtr;

class ImageStreamer2D : public Streamer {
    public:
        Image2DtPtr getOutput();

};

}; // end namespace fast

#endif
