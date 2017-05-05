#ifndef VTK_MESH_FILE_EXPORTER_HPP
#define VTK_MESH_FILE_EXPORTER_HPP

#include "FAST/Exporters/FileExporter.hpp"

namespace fast {

class VTKMeshFileExporter : public FileExporter {
    FAST_OBJECT(VTKMeshFileExporter);
    public:
        void setWriteNormals(bool writeNormals);
    private:
        VTKMeshFileExporter();
        void execute();

        bool mWriteNormals;
};

}

#endif
