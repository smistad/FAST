#pragma once

#include <FAST/Importers/FileImporter.hpp>

namespace fast {

/**
 * @brief Import image Pyramid stored as tiled multi-page TIFF
 *
 * To load a WSI image, use the WholeSlideImageImporter instead which
 * uses the openslide library.
 * This TIFFImagePyramidImporter is best used for important single channel
 * image pyramids such as high-res segmentations.
 * For instance if exported using the TIFFImagePyramidExporter.
 * This exporter uses libtiff.
 *
 * @sa WholeSlideImageImporter TIFFImagePyramidExporter
 * @ingroup importers
 */
class FAST_EXPORT TIFFImagePyramidImporter : public FileImporter {
    FAST_PROCESS_OBJECT(TIFFImagePyramidImporter)
    public:
        FAST_CONSTRUCTOR(TIFFImagePyramidImporter, std::string, filename,)
        void loadAttributes() override;
    private:
        TIFFImagePyramidImporter();
        void execute() override;
};

}