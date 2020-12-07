#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT LabelModifier : public ProcessObject {
    FAST_OBJECT(LabelModifier)
    public:
        void setLabelChange(uchar oldLabel, uchar newLabel);
        void loadAttributes() override;
    protected:
        LabelModifier();
        void execute() override;
        std::vector<uchar> m_labelChanges;
};

}