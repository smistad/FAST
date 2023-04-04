#pragma once

#include <FAST/Importers/FileImporter.hpp>

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
class FAST_EXPORT  ImageFileImporter : public FileImporter {
    FAST_PROCESS_OBJECT(ImageFileImporter)
    public:
        /**
         * @brief Create an instance
         *
         * @param filename Path to image file to load
         * @param grayscale Convert images to grayscale if the source image is in color
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageFileImporter, std::string, filename,,
                         bool, grayscale, = false);
        /**
         * @brief Convert images to grayscale if the source image is in color
         * @param grayscale Convert or not
         */
        void setGrayscale(bool grayscale);
        void loadAttributes() override;
    private:
        ImageFileImporter();
        void execute() override;
        void init();
        bool m_grayscale = false;
};

}