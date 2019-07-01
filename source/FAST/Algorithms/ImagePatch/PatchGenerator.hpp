#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Streamers/Streamer.hpp>
#include <thread>

namespace fast {

class FAST_EXPORT PatchGenerator : public Streamer {
    FAST_OBJECT(PatchGenerator)
    public:
        void setPatchSize(int width, int height);
        void setPatchLevel(int level);
        ~PatchGenerator();
    protected:
        int m_width, m_height;

        SharedPointer<WholeSlideImage> m_inputImage;
        int m_level;

        void execute();
        void generateStream() override;
    private:
        PatchGenerator();
};
}