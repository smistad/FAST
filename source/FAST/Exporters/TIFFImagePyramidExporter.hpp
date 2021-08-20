#pragma once

#include <FAST/Exporters/FileExporter.hpp>
#include <FAST/Data/ImagePyramid.hpp>

namespace fast {

/**
 * @brief Export an ImagePyramid to disk in the tiled pyramid TIFF format.
 *
 * @ingroup exporter wsi
 * @sa TIFFImagePyramidImporter
 */
class FAST_EXPORT TIFFImagePyramidExporter : public FileExporter {
    FAST_PROCESS_OBJECT(TIFFImagePyramidExporter)
public:
    FAST_CONSTRUCTOR(TIFFImagePyramidExporter,
                     std::string, filename,,
                     ImageCompression, compression, = ImageCompression::UNSPECIFIED
    )
    void setCompression(ImageCompression compression);
    void loadAttributes() override;
protected:
    TIFFImagePyramidExporter();
    void execute() override;

    bool m_compressionSet = false;
    ImageCompression m_compression;
};

}