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
        /**
         * Create instance
         *
         * @param filename Filename to export image to
         * @param compress Use lossless compression if possible (.mhd/.zraw)
         * @param resampleIfNeeded If image is not isotropic and target format is standard image (jpg,gif,bmp etc),
         *      image will be resampled first to be isotropic.
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageFileExporter,
                         std::string, filename,,
                         bool, compress, = false,
                         bool, resampleIfNeeded, = true
        )
        void setCompression(bool compress);
        void setResampleIfNeeded(bool resample);
    private:
        ImageFileExporter();
        void execute() override;

        bool mCompress = false;
        bool m_resample = true;
};

}

