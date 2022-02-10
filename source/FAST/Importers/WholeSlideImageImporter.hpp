#pragma once

#include <FAST/Importers/FileImporter.hpp>

namespace fast {

/**
 * @brief Read a whole slide image from disk
 *
 * This importer uses the [OpenSlide](https://openslide.org/) library to read whole slide images uses in digital pathology.
 * It can read the following formats:
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
 *
 * Outputs
 * - 0: ImagePyramid
 *
 * @ingroup importers wsi
 */
class FAST_EXPORT WholeSlideImageImporter : public FileImporter {
    FAST_PROCESS_OBJECT(WholeSlideImageImporter)
    public:
        FAST_CONSTRUCTOR(WholeSlideImageImporter, std::string, filename,)
        void loadAttributes() override;
    private:
        WholeSlideImageImporter();
        void readWithOpenSlide(std::string filename);
        void readVSI(std::string filename);
        void execute();
};

}
