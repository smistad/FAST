#pragma once

#include "Importer.hpp"
#include <string>

namespace fast {

/**
 * @brief Read JPG/PNG/BMP/TIFF images
 *
 * Uses Qt5 to read and output plain JPG/PNG/BMP/GIF images from disk.
 *
 * @ingroup importers
 */
class FAST_EXPORT ImageImporter : public Importer {
    FAST_OBJECT(ImageImporter)
    public:
        void setFilename(std::string filename);
        void setGrayscale(bool grayscale);
        void loadAttributes() override;
    private:
        ImageImporter();
        std::string mFilename;
        bool mGrayscale;
        void execute() override;
};

} // end namespace fast


