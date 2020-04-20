#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT WholeSlideImageImporter : public ProcessObject {
    FAST_OBJECT(WholeSlideImageImporter)
    public:
        void setFilename(std::string filename);
        WholeSlideImageImporter();
        void setGrayscale(bool grayscale);
        void loadAttributes() override;
    private:
        void execute();

        std::string mFilename;
        bool mGrayscale;
};

}
