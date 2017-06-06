#ifndef META_IMAGE_IMPORTER_HPP_
#define META_IMAGE_IMPORTER_HPP_

#include "Importer.hpp"

namespace fast {

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

#endif
