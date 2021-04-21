#pragma once

#include "FAST/ProcessObject.hpp"
#include <string>

namespace fast {

/**
 * @brief Export an Image to disk with the metaimage format (.mhd/.raw)
 *
 * This exporter writes 2D and 3D images using the MetaImage format which are pairs of .mhd text files and .raw files
 * containing raw pixel data.
 * Supports compression (.zraw) using the zlib library.
 * All meta data in the Image is stored in the .mhd text file.
 *
 * <h3>Input ports</h3>
 * 0: Image
 *
 * @ingroup exporters
 * @sa MetaImageImporter, ImageFileExporter
 */
class FAST_EXPORT MetaImageExporter : public ProcessObject {
    FAST_OBJECT(MetaImageExporter)
    public:
        void setFilename(std::string filename);
        /**
         * Enable or disable lossless compression
         * @param compress
         */
        void setCompression(bool compress);
        /**
         * Deprecated
         */
        void enableCompression();
        /**
         * Deprecated
         */
        void disableCompression();
        /**
         * Add additional meta data to the mhd file.
         * This can also be added to the input image object.
         *
         * @param key
         * @param value
         */
        void setMetadata(std::string key, std::string value);
    private:
        MetaImageExporter();
        void execute();

        std::string mFilename;
        std::map<std::string, std::string> mMetadata;
        bool mUseCompression;
};

} // end namespace fast