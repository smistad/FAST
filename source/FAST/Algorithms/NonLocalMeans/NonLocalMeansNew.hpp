#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {
    class FAST_EXPORT NonLocalMeansNew : public ProcessObject {
        FAST_OBJECT(NonLocalMeansNew);
    public:
    private:
        NonLocalMeansNew();
        void execute() override;
    };
}