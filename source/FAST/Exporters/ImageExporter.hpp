#pragma once

#include "FAST/ProcessObject.hpp"
#include <string>

namespace fast {

/**
 * @brief Write an Image to a file using image format such as JPG/PNG/BMP/GIF
 *
 * This exporter uses Qt 5 to write an Image to file using common image formats.
 * Check out [Qt 5 webpage for more details on formats supported](https://doc.qt.io/qt-5/qimage.html#reading-and-writing-image-files)
 *
 * <h3>Input ports</h3>
 * 0: Image
 *
 * @ingroup exporters
 * @sa ImageImporter
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
};


} // end namespace fast
