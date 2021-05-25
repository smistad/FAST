#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

enum class BoundingBoxNetworkType {
    YOLOv3,
};

class TensorToBoundingBoxSet : public ProcessObject {
    FAST_OBJECT(TensorToBoundingBoxSet)
    public:
        void setType(BoundingBoxNetworkType type);
    private:
        TensorToBoundingBoxSet();
        void execute() override;

        BoundingBoxNetworkType m_type;
};

}