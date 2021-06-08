#pragma once

#include <FAST/Importers/FileImporter.hpp>
#include <string>

namespace fast {

/**
 * @brief Read JPG/PNG/BMP/TIFF images
 *
 * Uses Qt5 to read and output plain JPG/PNG/BMP/GIF images from disk.
 *
 * @ingroup importers
 */
class FAST_EXPORT ImageImporter : public FileImporter {
    FAST_PROCESS_OBJECT(ImageImporter)
    public:
        FAST_CONSTRUCTOR(ImageImporter,
                         std::string, filename,,
                         bool, convertToGrayscale, = true
        )
        void setGrayscale(bool grayscale);
        void loadAttributes() override;
    private:
        ImageImporter();
        bool mGrayscale;
        void execute() override;
};

} // end namespace fast


