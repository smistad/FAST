#pragma once

#include <FAST/Streamers/Streamer.hpp>

namespace fast {

class Image;

class FAST_EXPORT ImageToBatchGenerator : public Streamer {
    FAST_OBJECT(ImageToBatchGenerator)
    public:
        void setMaxBatchSize(int size);
        bool hasReachedEnd() override;
    protected:
        void execute() override;
        int m_maxBatchSize;
        std::vector<SharedPointer<Image>> m_imageList;
    private:
        ImageToBatchGenerator();
};

}