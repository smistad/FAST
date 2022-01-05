#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {
/**
 * @brief Perform binary dilation with disk structuring element
 *
 * @ingroup morphology segmentation
 */
class FAST_EXPORT  Dilation : public ProcessObject {
    FAST_PROCESS_OBJECT(Dilation)
public:
    /**
     * @brief Create instance
     * @param size Size of square dilation structuring element. Default 3: 3x3
     * @return instance
     */
    FAST_CONSTRUCTOR(Dilation, int, size, = 3)
    /**
     * Set size of structuring element, must be odd
     * @param size
     */
    void setStructuringElementSize(int size);
    void loadAttributes() override;
private:
    void execute() override;

    int mSize;

};
}