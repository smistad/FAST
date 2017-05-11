#ifndef FAST_DILATION_HPP_
#define FAST_DILATION_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {
class FAST_EXPORT  Dilation : public ProcessObject {
    FAST_OBJECT(Dilation)
public:
    /**
     * Set size of structuring element, must be odd
     * @param size
     */
    void setStructuringElementSize(int size);
private:
    Dilation();
    void execute();

    int mSize;

};
}

#endif