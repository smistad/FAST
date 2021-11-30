#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT EnvelopeAndLogCompressor : public ProcessObject {
    FAST_PROCESS_OBJECT(EnvelopeAndLogCompressor)
    public:
        FAST_CONSTRUCTOR(EnvelopeAndLogCompressor)
    private:
        void execute() override;
};
}