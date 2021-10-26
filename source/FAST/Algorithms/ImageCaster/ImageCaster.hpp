#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Cast image to another data type
 *
 * @ingroup filter
 */
class FAST_EXPORT ImageCaster : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageCaster)
    public:
        FAST_CONSTRUCTOR(ImageCaster,
                         DataType, outputType,,
                         float, scaleFactor, = 1.0f
        )
    private:
        ImageCaster();
        void execute() override;
        float m_scaleFactor;
        DataType m_outputType;
};

}