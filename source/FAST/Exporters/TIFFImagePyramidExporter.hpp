#pragma once

#include <FAST/Exporters/FileExporter.hpp>

namespace fast {

/**
 * @brief Export an ImagePyramid to disk in the tiled pyramid TIFF format.
 */
class TIFFImagePyramidExporter : public FileExporter {
    FAST_OBJECT(TIFFImagePyramidExporter)
public:
    void loadAttributes() override;
protected:
    TIFFImagePyramidExporter();
    void execute() override;
};

}