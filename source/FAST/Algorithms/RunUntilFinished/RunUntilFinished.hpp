#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Calls run on parent PO until output is marked as last frame
 *
 * This PO is useful when having multi-step processsing and a previous step needs to run on
 * a stream of data before it is finished.
 *
 */
class FAST_EXPORT RunUntilFinished : public ProcessObject {
    FAST_PROCESS_OBJECT(RunUntilFinished)
    public:
        FAST_CONSTRUCTOR(RunUntilFinished);
    private:
        void execute() override;
};

}