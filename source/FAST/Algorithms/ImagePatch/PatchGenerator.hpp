#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Streamers/Streamer.hpp>
#include <thread>

namespace fast {

class ImagePyramid;
class Image;

class FAST_EXPORT PatchGenerator : public Streamer {
    FAST_OBJECT(PatchGenerator)
    public:
        void setPatchSize(int width, int height, int depth = 1);
        /**
         * Set overlap of generated patches, in percent of patch size.
         * @param percent
         */
        void setOverlap(float percent);
        void setPatchLevel(int level);
        void setMaskThreshold(float percent);
        ~PatchGenerator();
        void loadAttributes() override;
    protected:
        int m_width, m_height, m_depth;
        float m_overlapPercent = 0;
        float m_maskThreshold = 0.5;

        std::shared_ptr<ImagePyramid> m_inputImagePyramid;
        std::shared_ptr<Image> m_inputVolume;
        std::shared_ptr<Image> m_inputMask;
        int m_level;

        void execute() override;
        void generateStream() override;
    private:
        PatchGenerator();
};
}