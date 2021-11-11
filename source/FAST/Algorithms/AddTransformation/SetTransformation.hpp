#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/SpatialDataObject.hpp"

namespace fast {

/**
 * @brief Set transformations to spatial data objects
 *
 * This class will set the transform for spatial data object.
 *
 * Inputs:
 * - 0: SpatialDataObject - Object to set transform to
 * - 1: AffineTransformation - Transform to set
 *
 * Outputs:
 * - 0: SpatialDataObject - Same as input 0
 */
class FAST_EXPORT  SetTransformation : public ProcessObject {
    FAST_PROCESS_OBJECT(SetTransformation)
    public:
        /**
         * @brief Create instance
         * @return instance
         */
        FAST_CONSTRUCTOR(SetTransformation);
    private:
        void execute();
};

}