#pragma once

#include <FAST/Exporters/FileExporter.hpp>

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
class FAST_EXPORT ImageFileExporter : public FileExporter {
    FAST_PROCESS_OBJECT(ImageFileExporter)
    public:
        FAST_CONSTRUCTOR(ImageFileExporter,
                         std::string, filename,,
                         bool,compress,= false
        )
        void setCompression(bool compress);
    private:
        ImageFileExporter();
        void execute() override;

        bool mCompress = false;
};

}

