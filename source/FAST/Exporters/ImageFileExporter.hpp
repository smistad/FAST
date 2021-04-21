#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Write image to a file with various formats
 *
 * This exporter will look at the file extension and determine which image exporter should be used to read the file.
 * - .jpg, .jpeg, .png, .bmp, .gif -> ImageExporter
 * - .mhd -> MetaImageExporter
 *
 * <h3>Input ports</h3>
 * 0: Image
 *
 * @ingroup exporters
 * @sa ImageFileImporter
 */
class FAST_EXPORT ImageFileExporter : public ProcessObject {
    FAST_OBJECT(ImageFileExporter)
    public:
        void setFilename(std::string filename);
        ImageFileExporter();
        void setCompression(bool compress);
    private:
        void execute() override;

        std::string mFilename;
        bool mCompress = false;
};

}

