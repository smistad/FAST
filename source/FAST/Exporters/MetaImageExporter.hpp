#ifndef METAIMAGEEXPORTER_HPP_
#define METAIMAGEEXPORTER_HPP_

#include "FAST/ProcessObject.hpp"
#include <string>

namespace fast {

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



#endif /* METAIMAGEEXPORTER_HPP_ */
