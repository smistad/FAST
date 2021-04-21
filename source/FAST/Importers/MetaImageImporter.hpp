#pragma once

#include "Importer.hpp"

namespace fast {

/**
 * @brief Loads an image stored in the MetaImage format (.mhd/.raw)
 *
 * This importer loads 2D and 3D images stored in the MetaImage format which are pairs of .mhd text files and .raw files
 * contain the raw pixel data.
 * It supports the compressed .zraw format as well using zlib.
 * It also loads all meta data stored in the .mhd text file which can be retrived by Image::getMetaData
 *
 * @ingroup importers
 */
class FAST_EXPORT  MetaImageImporter : public Importer {
    FAST_OBJECT(MetaImageImporter)
    public:
        void setFilename(std::string filename);
    private:
        MetaImageImporter();
        std::string mFilename;
        void execute();
};

} // end namespace fast
