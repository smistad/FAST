#pragma once

#include <FAST/Streamers/Streamer.hpp>

namespace fast {

class ImageToBatchGenerator : public Streamer {
    FAST_OBJECT(ImageToBatchGenerator)
    public:
        void execute();
    protected:
    private:
        ImageToBatchGenerator();
};

}