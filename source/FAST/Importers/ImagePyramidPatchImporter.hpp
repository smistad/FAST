#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * Imports an ImagePyramid stored as a set of image patches stored in a folder.
 */
class ImagePyramidPatchImporter : public ProcessObject {
    FAST_OBJECT(ImagePyramidPatchImporter)
    public:
        void setPath(std::string path);
        void loadAttributes() override;
    private:
        ImagePyramidPatchImporter();
        void execute() override;

        std::string m_path;
};

}