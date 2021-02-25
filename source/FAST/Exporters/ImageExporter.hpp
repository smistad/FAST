#pragma once

#include "FAST/ProcessObject.hpp"
#include <string>

namespace fast {

/**
 * Export an image to a normal image format such as PNG, JPEG, BMP etc.
 */
class FAST_EXPORT ImageExporter : public ProcessObject {
    FAST_OBJECT(ImageExporter)
    public:
        void setFilename(std::string filename);
    private:
        ImageExporter();
        void execute();

        std::string mFilename;
};


} // end namespace fast
