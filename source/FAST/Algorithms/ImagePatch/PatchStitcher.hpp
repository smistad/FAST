#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class Image;

class FAST_EXPORT PatchStitcher : public ProcessObject {
    FAST_OBJECT(PatchStitcher)
    public:
    protected:
        void execute() override;

        SharedPointer<Image> m_outputImage;
    private:
        PatchStitcher();

};

}