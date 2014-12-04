#ifndef DOUBLEFILTER_HPP
#define DOUBLEFILTER_HPP

#include "ProcessObject.hpp"
#include "Image.hpp"

namespace fast {

/**
 * This is an example filter which doubles the value of each element in an image
 */
class DoubleFilter : public ProcessObject {
    FAST_OBJECT(DoubleFilter)
    public:
        void setInput(ImageData::pointer image);
        ImageData::pointer getOutput();
    private:
        DoubleFilter();
        // This method will execute the algorithm
        void execute();
};

}; // namespace fast

#endif // DOUBLEFILTER_HPP
