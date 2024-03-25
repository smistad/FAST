#pragma once

#include <FAST/Importers/FileImporter.hpp>

namespace fast {

/**
 * @brief Read a whole slide image from disk
 *
 * This importer uses the [OpenSlide](https://openslide.org/) library to read whole slide images used in digital pathology,
 * in addition to support for the OME-TIFF format which is implemented in FAST using libtiff.
 * Thus, this importer can read the following formats:
 *
 * - Aperio (.svs, .tif)
 * - Hamamatsu (.vms, .vmu, .ndpi)
 * - Leica (.scn)
 * - MIRAX (.mrxs)
 * - Philips (.tiff)
 * - Sakura (.svslide)
 * - Trestle (.tif)
 * - Ventana (.bif, .tif)
 * - Generic tiled TIFF (.tif)
 * - OME-TIFF (ome.tiff, ome.tif, ome.btf)
 *
 * Outputs
 * - 0: ImagePyramid
 *
 * @ingroup importers wsi
 * @sa TIFFImagePyramidImporter
 */
class FAST_EXPORT WholeSlideImageImporter : public FileImporter {
    FAST_PROCESS_OBJECT(WholeSlideImageImporter)
    public:
        FAST_CONSTRUCTOR(WholeSlideImageImporter, std::string, filename,)
        void loadAttributes() override;
    private:
        WholeSlideImageImporter();
        void readWithOpenSlide(std::string filename);
        void readWithTIFF(std::string filename);
        void execute();
};

}
