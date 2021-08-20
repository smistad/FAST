#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {
/**
 * @brief Change labels in a segmentation image
 *
 * Used to converting all pixels with label/intensity X to label/intensity Y
 * @ingroup segmentation
 */
class FAST_EXPORT LabelModifier : public ProcessObject {
    FAST_PROCESS_OBJECT(LabelModifier)
    public:
        FAST_CONSTRUCTOR(LabelModifier, std::vector<uchar>, oldLabels,, std::vector<uchar>, newLabels,);
        void setLabelChange(uchar oldLabel, uchar newLabel);
        void loadAttributes() override;
    protected:
        LabelModifier();
        void execute() override;
        std::vector<uchar> m_labelChanges;
};

}