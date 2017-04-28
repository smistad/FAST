#ifndef VTK_MESH_FILE_EXPORTER_HPP
#define VTK_MESH_FILE_EXPORTER_HPP

#include "FAST/Exporters/FileExporter.hpp"

namespace fast {

class VTKMeshFileExporter : public FileExporter {
    FAST_OBJECT(VTKMeshFileExporter);
    private:
        VTKMeshFileExporter();
        void execute();
};

}

#endif
