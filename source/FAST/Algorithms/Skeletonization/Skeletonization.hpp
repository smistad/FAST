#ifndef SKELETONIZATION_HPP
#define SKELETONIZATION_HPP

#include "FAST/ProcessObject.hpp"

namespace fast {

class Skeletonization : public ProcessObject {
    FAST_OBJECT(Skeletonization)
    public:
    private:
        Skeletonization();
        void execute();
};

} // end namespace fast

#endif
