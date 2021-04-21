#pragma once

#include "FAST/Exporters/FileExporter.hpp"

namespace fast {

/**
 * @brief Write Mesh to file using the VTK polydata format
 *
 * <h3>Input ports</h3>
 * - 0: Mesh
 *
 * @sa VTKMeshFileImporter
 * @ingroup exporters
 */
class FAST_EXPORT  VTKMeshFileExporter : public FileExporter {
    FAST_OBJECT(VTKMeshFileExporter);
    public:
        void setWriteNormals(bool writeNormals);
        void setWriteColors(bool writeColors);
    private:
        VTKMeshFileExporter();
        void execute();

        bool mWriteNormals;
        bool mWriteColors;
};

}