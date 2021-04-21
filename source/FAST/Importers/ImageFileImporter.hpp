#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Read image data stored in various file formats.
 *
 * This importer will look at the file extension and determine which image importer should be used to read the file.
 * - .jpg, .jpeg, .png, .bmp, .gif -> ImageImporter
 * - .dcm (or no extension) -> DICOMFileImporter
 * - .mhd -> MetaImageImporter
 *
 * <h3>Output ports</h3>
 * 0: Image
 *
 * @ingroup importers
 */
class FAST_EXPORT  ImageFileImporter : public ProcessObject {
    FAST_OBJECT(ImageFileImporter)
    public:
        void setFilename(std::string filename);
        void loadAttributes() override;
    private:
        ImageFileImporter();
        void execute() override;

        std::string mFilename;
};

}