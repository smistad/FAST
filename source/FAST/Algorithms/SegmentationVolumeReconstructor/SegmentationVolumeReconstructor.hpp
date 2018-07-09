#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class Segmentation;

class SegmentationVolumeReconstructor : public ProcessObject {
    FAST_OBJECT(SegmentationVolumeReconstructor)
    public:
    private:
        SegmentationVolumeReconstructor();
        void execute() override;

        std::shared_ptr<Segmentation> m_volume;
};

}