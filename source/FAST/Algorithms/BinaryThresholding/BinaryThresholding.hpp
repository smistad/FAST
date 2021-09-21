#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Segmentation using a threshold value
 *
 * Any pixel with intensity between lower and upper threshold is labeled as 1, the rest is background 0.
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs:
 * - 0: Image segmentation
 *
 * @ingroup segmentation
 */
class FAST_EXPORT BinaryThresholding : public ProcessObject {
    FAST_PROCESS_OBJECT(BinaryThresholding)
    public:
        /**
         * @brief Create instance
         * @param lowerThreshold All pixels with an intensity above this threshold will be segmented
         * @param upperThreshold All pixels with intensity below this threshold will be segmented
         * @return instance
         */
        FAST_CONSTRUCTOR(BinaryThresholding,
                         float, lowerThreshold,,
                         float, upperThreshold, = std::numeric_limits<float>::max()
        );
        void setLowerThreshold(float threshold);
        void setUpperThreshold(float threshold);
        void loadAttributes() override;
    private:
        BinaryThresholding();

        void execute();

        void waitToFinish();

        float mLowerThreshold;
        float mUpperThreshold;
        bool mLowerThresholdSet;
        bool mUpperThresholdSet;
};

}
