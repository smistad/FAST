#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class Image;
class ImagePyramid;
class Tensor;

class FAST_EXPORT PatchStitcher : public ProcessObject {
    FAST_OBJECT(PatchStitcher)
    public:
    protected:
        void execute() override;

        std::shared_ptr<Image> m_outputImage;
        std::shared_ptr<Tensor> m_outputTensor;
        std::shared_ptr<ImagePyramid> m_outputImagePyramid;

        void processTensor(std::shared_ptr<Tensor> tensor);
        void processImage(std::shared_ptr<Image> tensor);
    private:
        PatchStitcher();

};

}