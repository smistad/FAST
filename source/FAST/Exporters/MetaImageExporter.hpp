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
    private:
        MetaImageExporter();
        void execute();

        std::string mFilename;
        bool mUseCompression;
};

} // end namespace fast



#endif /* METAIMAGEEXPORTER_HPP_ */
