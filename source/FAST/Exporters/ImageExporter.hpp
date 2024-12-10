#pragma once

#include <FAST/Exporters/FileExporter.hpp>
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
class FAST_EXPORT ImageExporter : public FileExporter {
    FAST_PROCESS_OBJECT(ImageExporter)
    public:
        /**
         * @brief Create instance
         * @param filename
         * @param quality When lossy compression is used like JPEG or JPEGXL, this parameter controls the quality vs size.
         *      100 = best, 0 = worst.
         * @param resampleIfNeeded
         * @return Instance
         */
        FAST_CONSTRUCTOR(ImageExporter,
                         std::string, filename,,
                         int, quality, = 90,
                         bool, resampleIfNeeded, = true)
        void setResampleIfNeeded(bool resample);
        void setQuality(int quality);
        void loadAttributes() override;
    private:
        ImageExporter();
        void execute() override;

        bool m_resample;
        int m_quality = 90;
};


} // end namespace fast
