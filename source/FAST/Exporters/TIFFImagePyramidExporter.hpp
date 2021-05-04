#pragma once

#include <FAST/Exporters/FileExporter.hpp>

namespace fast {

/**
 * @brief Image compression types for TIFF exporter
 */
enum class ImageCompression {
    RAW,
    JPEG,
    JPEG2000,
    LZW
};

/**
 * @brief Export an ImagePyramid to disk in the tiled pyramid TIFF format.
 */
class TIFFImagePyramidExporter : public FileExporter {
    FAST_OBJECT(TIFFImagePyramidExporter)
public:
    void setCompression(ImageCompression compression);
    void loadAttributes() override;
protected:
    TIFFImagePyramidExporter();
    void execute() override;

    bool m_compressionSet = false;
    ImageCompression m_compression;
};

}