#ifndef IMAGE_STREAMER_2D_HPP
#define IMAGE_STREAMER_2D_HPP

#include "Streamer.hpp"
#include "Image2Dt.hpp"
#include <boost/shared_ptr.hpp>

namespace fast {

class ImageStreamer2D : public Streamer {
    FAST_OBJECT(ImageStreamer2D)
    public:
        Image2Dt::Ptr getOutput();
    private:
        // A reference to the output object used to update its next frame
        Image2Dt::Ptr mOutput;

        // Update the streamer if any parameters have changed
        void execute();

        // This method runs in a separate thread and adds frames to the
        // output object
        void producerStream();

};

} // end namespace fast

#endif
