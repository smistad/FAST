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
    private:
        Erosion();
        void execute();

        int mSize;

    };
}
