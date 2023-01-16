#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {
class FAST_EXPORT GrayscaleToColor : public ProcessObject {
    FAST_PROCESS_OBJECT(GrayscaleToColor)
    public:
        FAST_CONSTRUCTOR(GrayscaleToColor, bool, addAlphaChannel, = false)
    protected:
        void execute() override;

        bool m_addAlphaChannel = false;
};
}
