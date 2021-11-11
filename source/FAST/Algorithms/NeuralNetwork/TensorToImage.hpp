#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Converts a Tensor with shape HxWxC to a FAST Image
 *
 * @ingroup neural-network
 */
class FAST_EXPORT TensorToImage : public ProcessObject {
    FAST_PROCESS_OBJECT(TensorToImage)
    public:
        /**
         * @brief Create instance
         * @param channels which channels to extract from tensor.
         *      If empty all channels are used. Nr of channels has to be <= 4
         * @return instance
         */
        FAST_CONSTRUCTOR(TensorToImage, std::vector<int>, channels, = std::vector<int>())
        void setChannels(std::vector<int> channels);
        void loadAttributes();
    protected:
        void execute() override;
        std::vector<int> m_channels;
};

}