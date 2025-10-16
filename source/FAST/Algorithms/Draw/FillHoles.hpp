#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Fill holes in image/segmentation
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs
 * - 0: Image
 *
 * @ingroup draw
 */
class FAST_EXPORT FillHoles : public ProcessObject {
    FAST_OBJECT(FillHoles)
    public:
        FAST_CONSTRUCTOR(FillHoles)
    private:
        void execute() override;
};
}