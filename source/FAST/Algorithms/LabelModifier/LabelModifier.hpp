#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {
/**
 * @brief Change labels in a segmentation image
 *
 * Used to converting all pixels with label/intensity X to label/intensity Y
 *
 * @todo Use std::map/dict in constructor instead of two vectors?
 *
 * @ingroup segmentation
 */
class FAST_EXPORT LabelModifier : public ProcessObject {
    FAST_PROCESS_OBJECT(LabelModifier)
    public:
        /**
         * @brief Create instance
         * @param oldLabels A list of labels to change
         * @param newLabels A list of new labels for the labels specified in oldLabels
         * @return
         */
        FAST_CONSTRUCTOR(LabelModifier,
                         std::vector<uchar>, oldLabels,,
                         std::vector<uchar>, newLabels,
        );
        void setLabelChange(uchar oldLabel, uchar newLabel);
        void loadAttributes() override;
    protected:
        LabelModifier();
        void execute() override;
        std::vector<uchar> m_labelChanges;
};

}