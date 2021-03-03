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
    void loadAttributes() override;
private:
    Dilation();
    void execute() override;

    int mSize;

};
}

#endif