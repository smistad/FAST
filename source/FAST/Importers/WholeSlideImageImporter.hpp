#ifndef IMAGE_FILE_IMPORTER_HPP_
#define IMAGE_FILE_IMPORTER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT WholeSlideImageImporter : public ProcessObject {
    FAST_OBJECT(WholeSlideImageImporter)
    public:
        void setFilename(std::string filename);
        WholeSlideImageImporter();
        void setGrayscale(bool grayscale);
    private:
        void execute();

        std::string mFilename;
        bool mGrayscale;
};

}

#endif
