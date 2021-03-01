#pragma once

#include "Importer.hpp"
#include <string>

namespace fast {

class FAST_EXPORT  ImageImporter : public Importer {
    FAST_OBJECT(ImageImporter)
    public:
        void setFilename(std::string filename);
        void setGrayscale(bool grayscale);
        void loadAttributes() override;
    private:
        ImageImporter();
        std::string mFilename;
        bool mGrayscale;
        void execute();

};


} // end namespace fast


