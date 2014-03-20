#ifndef IMAGE_STREAMER_2D_HPP
#define IMAGE_STREAMER_2D_HPP

#include "Streamer.hpp"
#include "Image2Dt.hpp"

namespace fast {

class ImageStreamer2D : public Streamer {
    public:
        Image2DtPtr getOutput();
    private:
        // A reference to the output object used to update its next frame
        Image2DtPtr mOutput;

        // Update the streamer if any parameters have changed
        void execute();

        // This method runs in a separate thread and adds frames to the
        // output object
        void producerStream();

};

typedef boost::shared_ptr<ImageStreamer2D> ImageStreamer2DPtr;

}; // end namespace fast

#endif
