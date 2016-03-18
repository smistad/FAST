#ifndef GAUSSIAN_FILTERING_HPP_
#define GAUSSIAN_FILTERING_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Algorithms/Filtering/Filtering.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

    class GaussianFiltering : public Filtering {
        FAST_OBJECT(GaussianFiltering)
    public:
        void setStandardDeviation(float stdDev);
        ~GaussianFiltering();
    protected:
        GaussianFiltering();
        bool isSeparable();
        float * getSeparable(int dir);
        float createMaskValue(int x, int y, int z = 0);
        
        float mStdDev;
    };

} // end namespace fast

#endif /* GAUSSIAN_FILTERING_HPP_ */