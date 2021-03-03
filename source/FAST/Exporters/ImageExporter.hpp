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
        void loadAttributes() override;
    private:
        ImageExporter();
        void execute() override;

        std::string mFilename;
        bool mIsModified = true;
};


} // end namespace fast
