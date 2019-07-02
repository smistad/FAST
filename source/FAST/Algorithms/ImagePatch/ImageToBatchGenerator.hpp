#pragma once

#include <FAST/Streamers/Streamer.hpp>
#include <thread>

namespace fast {

class Image;

class FAST_EXPORT ImageToBatchGenerator : public Streamer {
    FAST_OBJECT(ImageToBatchGenerator)
    public:
        void setMaxBatchSize(int size);
        ~ImageToBatchGenerator() override;
        void stop();
    protected:
        void execute() override;
        void generateStream();
        int m_maxBatchSize;

        DataChannel::pointer mParent;
    private:
        ImageToBatchGenerator();
};

}