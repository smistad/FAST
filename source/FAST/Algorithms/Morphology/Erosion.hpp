#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {
    class FAST_EXPORT Erosion : public ProcessObject {
    FAST_OBJECT(Erosion)
    public:
        /**
         * Set size of structuring element, must be odd
         * @param size
         */
        void setStructuringElementSize(int size);
        void loadAttributes() override;
    private:
        Erosion();
        void execute() override;

        int mSize;

    };
}
