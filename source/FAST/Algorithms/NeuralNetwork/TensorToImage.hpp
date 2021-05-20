#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Converts a Tensor with shape HxWxC to a FAST Image
 *
 * @ingroup neural-network
 */
class TensorToImage : public ProcessObject {
    FAST_OBJECT(TensorToImage)
    public:
    protected:
        TensorToImage();
        void execute() override;
};

}