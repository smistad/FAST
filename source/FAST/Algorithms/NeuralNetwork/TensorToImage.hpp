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
        FAST_CONSTRUCTOR(TensorToImage)
    protected:
        void execute() override;
};

}