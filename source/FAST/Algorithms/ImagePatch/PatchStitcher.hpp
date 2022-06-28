#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class Image;
class ImagePyramid;
class Tensor;

/**
 * @brief Stitch a stream of processed patches from the PatchGenerator
 *
 * This process object stitches a stream of processed Image/Tensor patches into an
 * ImagePyramid, 2D or 3D Image or Tensor depending on the patch source.
 *
 * Inputs:
 * 0 - Image/Tensor: A stream of processed patches from PatchGenerator
 * Outputs:
 * 0 - ImagePyramid/Image/Tensor: The stitched image/image pyramid.
 *
 * @ingroup wsi
 * @sa PatchGenerator
 */
class FAST_EXPORT PatchStitcher : public ProcessObject {
    FAST_PROCESS_OBJECT(PatchStitcher)
    public:
        /**
         * @brief Create instance
         * @return instance
         */
        FAST_CONSTRUCTOR(PatchStitcher, bool, patchesAreCropped, = false);
        void loadAttributes() override;
        /**
         * @brief Set whether incoming patches are cropped or not
         * @param cropped
         */
        void setPatchesAreCropped(bool cropped);
        /**
         * @brief Get whether incoming patches are cropped or not
         * @param cropped
         */
        bool getPatchesAreCropped() const;
    protected:
        void execute() override;

        std::shared_ptr<Image> m_outputImage;
        std::shared_ptr<Tensor> m_outputTensor;
        std::shared_ptr<ImagePyramid> m_outputImagePyramid;

        void processTensor(std::shared_ptr<Tensor> tensor);
        void processImage(std::shared_ptr<Image> tensor);
    private:
        bool m_patchesAreCropped = false;

};

}