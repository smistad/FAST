#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT TensorToSegmentation : public ProcessObject {
    FAST_OBJECT(TensorToSegmentation)
    public:
    protected:
        TensorToSegmentation();
        void execute() override;
        float m_threshold = 0.5f;
};

}