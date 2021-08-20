#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

/**
 * @brief Reconstruct a stream of 2D segmentation images into a 3D segmentation image
 *
 * @ingroup segmentation
 */
class FAST_EXPORT SegmentationVolumeReconstructor : public ProcessObject {
    FAST_PROCESS_OBJECT(SegmentationVolumeReconstructor)
    public:
        FAST_CONSTRUCTOR(SegmentationVolumeReconstructor)
    private:
        void execute() override;

        std::shared_ptr<Image> m_volume;
};

}