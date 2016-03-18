#ifndef SOBEL_FILTERING_HPP_
#define SOBEL_FILTERING_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Algorithms/Filtering/Filtering.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

    class SobelFiltering : public Filtering {
        FAST_OBJECT(SobelFiltering)
    public:
        void setDirection(int direction);
        float createMaskValue(int x, int y, int z = 1); //Special for sobel, if no z, use z=1
        ~SobelFiltering();
    protected:
        SobelFiltering();
        bool isSeparable();
        float * getSeparable(int dir);

        int mDirection;
    };

} // end namespace fast

#endif /* SOBEL_FILTERING_HPP_ */