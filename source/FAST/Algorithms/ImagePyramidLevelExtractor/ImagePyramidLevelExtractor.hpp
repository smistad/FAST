#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Extract a level of an image pyramid as an image
 *
 * @ingroup wsi
 */
class FAST_EXPORT ImagePyramidLevelExtractor : public ProcessObject {
    FAST_PROCESS_OBJECT(ImagePyramidLevelExtractor)
    public:
        FAST_CONSTRUCTOR(ImagePyramidLevelExtractor, int, level, = -1);
        void setLevel(int level);
        void loadAttributes();
    private:
        void execute() override;

        int m_level;
};

}