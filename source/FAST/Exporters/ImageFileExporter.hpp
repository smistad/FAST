#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT ImageFileExporter : public ProcessObject {
    FAST_OBJECT(ImageFileExporter)
    public:
        void setFilename(std::string filename);
        ImageFileExporter();
        void setCompression(bool compress);
    private:
        void execute() override;

        std::string mFilename;
        bool mCompress = false;
};

}

