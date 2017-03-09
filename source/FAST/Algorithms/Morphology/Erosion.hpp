#ifndef FAST_EROSION_HPP_
#define FAST_EROSION_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {
    class Erosion : public ProcessObject {
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

#endif
