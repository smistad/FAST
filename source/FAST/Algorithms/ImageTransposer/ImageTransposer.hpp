#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Transpose images
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs:
 * - 0: Image transposed
 *
 * @ingroup filter
 */
class FAST_EXPORT ImageTransposer : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageTransposer)
    public:
        /**
         * @brief Create instance
         * @param axes Specify axes to swap for 3D images. If not supplied, X and Y will be transposed.
         * @return  instance
         */
        FAST_CONSTRUCTOR(ImageTransposer, std::vector<int>, axes, = std::vector<int>())
        void setAxes(std::vector<int> axes);
        void loadAttributes() override;
    private:
        void execute() override;

        std::vector<int> m_axes;
};

}