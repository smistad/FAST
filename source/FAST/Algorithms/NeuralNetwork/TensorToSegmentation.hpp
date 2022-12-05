#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief A process object which converts a Tensor to a Segmentation object.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT TensorToSegmentation : public ProcessObject {
    FAST_PROCESS_OBJECT(TensorToSegmentation)
    public:
        /**
         * @brief Create instance
         * @param threshold The minimum value of the class confidence value to be accepted. Default: 0.5
         * @param hasBackgroundClass Whether the neural network has a channel 0 which represents the "background". Default: true
         * @param channelsToIgnore List of output channels to ignore
         * @return instance
         */
        FAST_CONSTRUCTOR(TensorToSegmentation,
                         float, threshold, = 0.5f,
                         bool, hasBackgroundClass, = true,
                         std::vector<int>, channelsToIgnore, = std::vector<int>()
        )
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
        /**
         * @brief Specify list of output channels to ignore
         * @param channels
         */
        void setChannelsToIgnore(std::vector<int> channels);
        void loadAttributes();
    protected:
        void execute() override;
        float m_threshold = 0.5f;
        bool m_hasBackgroundClass = true;
        std::set<int> m_channelsToIgnore;
};

}