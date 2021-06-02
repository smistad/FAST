#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief A process object which converts a Tensor to a Segmentation object.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT TensorToSegmentation : public ProcessObject {
    FAST_OBJECT(TensorToSegmentation)
    public:
        /**
         * Threshold to accept a channel X as being class X.
         *
         * @param threshold
         */
        void setThreshold(float threshold);
        float getThreshold() const;
        /**
         * Set whether channel 0 of segmentation tensor is the "background" class, thereby getting the label 0 in the
         * resulting Segmentation.
         *
         * @param hasBackgroundClass
         */
        void setBackgroundClass(bool hasBackgroundClass);
        void loadAttributes();
    protected:
        TensorToSegmentation();
        void execute() override;
        float m_threshold = 0.5f;
        bool m_hasBackgroundClass = true;
};

}