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
 * - Output 0: ImagePyramid
 *
 * @ingroup importers
 */
class FAST_EXPORT WholeSlideImageImporter : public FileImporter {
    FAST_OBJECT(WholeSlideImageImporter)
    public:
        WholeSlideImageImporter();
        void loadAttributes() override;
    private:
        void readWithOpenSlide(std::string filename);
        void execute();
};

}
