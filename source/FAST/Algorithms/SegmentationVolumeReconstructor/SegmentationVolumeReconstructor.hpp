#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

class FAST_EXPORT SegmentationVolumeReconstructor : public ProcessObject {
    FAST_OBJECT(SegmentationVolumeReconstructor)
    public:
    private:
        SegmentationVolumeReconstructor();
        void execute() override;

        std::shared_ptr<Image> m_volume;
};

}