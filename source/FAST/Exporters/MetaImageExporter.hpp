#ifndef METAIMAGEEXPORTER_HPP_
#define METAIMAGEEXPORTER_HPP_

#include "FAST/ProcessObject.hpp"
#include <string>

namespace fast {

class FAST_EXPORT  MetaImageExporter : public ProcessObject {
    FAST_OBJECT(MetaImageExporter)
    public:
        void setFilename(std::string filename);
        void enableCompression();
        void disableCompression();
        /**
         * Add additional meta data to the mhd file
         * @param key
         * @param value
         */
        void setMetaData(std::string key, std::string value);
    private:
        MetaImageExporter();
        void execute();

        std::string mFilename;
        std::map<std::string, std::string> mMetaData;
        bool mUseCompression;
};

} // end namespace fast



#endif /* METAIMAGEEXPORTER_HPP_ */
