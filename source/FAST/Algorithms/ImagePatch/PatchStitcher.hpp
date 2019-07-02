#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class Image;
class Tensor;

class FAST_EXPORT PatchStitcher : public ProcessObject {
    FAST_OBJECT(PatchStitcher)
    public:
    protected:
        void execute() override;

        SharedPointer<Image> m_outputImage;
        SharedPointer<Tensor> m_outputTensor;

        void processTensor(SharedPointer<Tensor> tensor);
        void processImage(SharedPointer<Image> tensor);
    private:
        PatchStitcher();

};

}