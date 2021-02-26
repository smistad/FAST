#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * Imports an ImagePyramid stored as a set of image patches stored in a folder.
 * Each patch is stored as a PNG image with the file name indicating its position and size.
 *
 * @sa ImagePyramidPatchExporter
 */
class FAST_EXPORT ImagePyramidPatchImporter : public ProcessObject {
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