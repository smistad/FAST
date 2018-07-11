#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class Segmentation;

class FAST_EXPORT SegmentationVolumeReconstructor : public ProcessObject {
    FAST_OBJECT(SegmentationVolumeReconstructor)
    public:
    private:
        SegmentationVolumeReconstructor();
        void execute() override;

        SharedPointer<Segmentation> m_volume;
};

}