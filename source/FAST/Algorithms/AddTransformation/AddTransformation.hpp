#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/SpatialDataObject.hpp"

namespace fast {

/**
 * @brief Add transformations to spatial data objects
 *
 * This class will add new scene graph node before the root node of the input data object.
 * This node will get the transformation supplied to the transformation input connection
 *
 * Inputs:
 * - 0: SpatialDataObject - Object to add transform to
 * - 1: AffineTransformation - Transform to add
 *
 * Outputs:
 * - 0: SpatialDataObject - Same as input 0
 */
class FAST_EXPORT AddTransformation : public ProcessObject {
    FAST_PROCESS_OBJECT(AddTransformation)
    public:
        /**
         * @brief Create instance
         * @return instance
         */
        FAST_CONSTRUCTOR(AddTransformation);
    private:
        void execute();

        SpatialDataObject::pointer mPrevious;
};

}