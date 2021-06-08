#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Loads an image pyramid from a set of image patches stored in a folder.
 *
 * Imports an ImagePyramid stored as a set of image patches stored in a folder.
 * Each patch is stored as a PNG image with the file name indicating its position and size.
 *
 * @sa ImagePyramidPatchExporter
 * @ingroup importers
 */
class FAST_EXPORT ImagePyramidPatchImporter : public ProcessObject {
    FAST_PROCESS_OBJECT(ImagePyramidPatchImporter)
    public:
        FAST_CONSTRUCTOR(ImagePyramidPatchImporter, std::string, path,)
        void setPath(std::string path);
        void loadAttributes() override;
    private:
        ImagePyramidPatchImporter();
        void execute() override;

        std::string m_path;
};

}