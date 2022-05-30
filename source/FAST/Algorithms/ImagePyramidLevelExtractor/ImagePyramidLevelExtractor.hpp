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
    /**
     * @brief Create instance
     * @param level Specify which level to extract from image pyramid. Negative level means last level,
     *      e.g. lowest resolution(default).
     * @param magnification Which magnification to extract patches from.
     *      Setting this value for instance to 20, will trigger a search through all levels
     *      to find the image pyramid level which is closest to 20X magnification, 0.0005 mm pixel spacing.
     *      If no such level exist an exception is thrown.
     *      This parameter overrides the level parameter
     * @return instance
     */
        FAST_CONSTRUCTOR(ImagePyramidLevelExtractor, int, level, = -1, int, magnification, = -1);
        void setLevel(int level);
        void setMagnification(int magnification);
        void loadAttributes();
    private:
        void execute() override;

        int m_level;
        int m_magnification;
};

}