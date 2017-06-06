#ifndef VTK_MESH_FILE_EXPORTER_HPP
#define VTK_MESH_FILE_EXPORTER_HPP

#include "FAST/Exporters/FileExporter.hpp"

namespace fast {

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

#endif
