#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT ColorToGrayscale : public ProcessObject {
    FAST_PROCESS_OBJECT(ColorToGrayscale)
    public:
        FAST_CONSTRUCTOR(ColorToGrayscale)
    protected:
        void execute() override;
};
}