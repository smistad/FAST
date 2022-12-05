#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Streamers/Streamer.hpp>
#include <thread>

namespace fast {

class ImagePyramid;
class Image;

/**
 * @brief Generates a stream of patches from an ImagePyramid or 3D Image
 *
 * The patch generator tiles an ImagePyramid or 3D image into equal sized patches.
 * Although the patches can be smaller at the edges if the image size is not dividable by the patch size.
 * The result of the processed patches can be stitched together again to form a full
 * ImagePyramid/3D Image/Tensor by using the PatchStitcher.
 *
 * @ingroup wsi
 * @sa PatchStitcher
 */
class FAST_EXPORT PatchGenerator : public Streamer {
    FAST_PROCESS_OBJECT(PatchGenerator)
    public:
        /**
         * @brief Creates a PatchGenerator instance
         * @param width Width of patch (Note: patches can be smaller at boundaries and will be padded)
         * @param height Height of patch (Note: patches can be smaller at boundaries and will be padded)
         * @param depth Depth of patch (Note: patches can be smaller at boundaries and will be padded)
         * @param level Which level of an ImagePyramid to generate patches from.
         * @param magnification Which magnification to extract patches from.
         *      Setting this value for instance to 20, will trigger a search through all levels
         *      to find the image pyramid level which is closest to 20X magnification, 0.0005 mm pixel spacing.
         *      If no such level exist an exception is thrown.
         *      This parameter overrides the level parameter.
         * @param overlapPercent Amount of patch overlap in percent.
         * @param maskThreshold Threshold to accept a patch if the additional mask is provided.
         * @param paddingValue Value to pad patches with when out-of-bounds. Default is negative, meaning it will use
         *  (white)255 for color images, and (black)0 for grayscale images
         * @return instance
         */
        FAST_CONSTRUCTOR(PatchGenerator,
                         int, width,,
                         int, height,,
                         int, depth, = 1,
                         int, level, = 0,
                         int, magnification, = -1,
                         float, overlapPercent, = 0.0f,
                         float, maskThreshold, = 0.5f,
                         int, paddingValue, = -1
        )
        void setPatchSize(int width, int height, int depth = 1);
        /**
         * Set overlap of generated patches, in percent of patch size.
         * @param percent
         */
        void setOverlap(float percent);
        void setPatchLevel(int level);
        void setPatchMagnification(int magnification);
        void setMaskThreshold(float percent);
        void setPaddingValue(int paddingValue);
        ~PatchGenerator();
        void loadAttributes() override;
        /**
         * @brief Get progress of this patch generator in percent.
         * @return progress in percent 0.0-1.0
         */
        float getProgress();
    protected:
        int m_width, m_height, m_depth;
        float m_overlapPercent = 0;
        float m_maskThreshold = 0.5;
        int m_paddingValue = -1;
        int m_magnification = -1;
        float m_progress = 0.0f;

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