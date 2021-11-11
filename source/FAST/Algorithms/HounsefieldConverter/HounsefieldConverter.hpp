#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

/**
 * @brief Converts input image to Hounsefield units (if needed)
 *
 * Hounsefield units used in CT are defined as signed 16 bit integers from -1024 and up.
 * This PO simply converts UINT16 input images to INT16 and subtracts 1024 from each voxel.
 * If the input image already is INT16, it will only pass to input to the output.
 *
 * Inputs:
 * - 0: Image 3D INT16 or UINT16
 *
 * Outputs:
 * - 0: Image 3D INT16
 */
class FAST_EXPORT HounsefieldConverter : public ProcessObject {
    FAST_PROCESS_OBJECT(HounsefieldConverter)
    public:
        /**
         * @brief Create instance
         * @return instance
         */
        FAST_CONSTRUCTOR(HounsefieldConverter);
    private:
        void execute();
        std::shared_ptr<Image> convertToHU(std::shared_ptr<Image> image);
};

}
