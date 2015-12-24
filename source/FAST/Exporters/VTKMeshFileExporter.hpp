#ifndef VTK_MESH_FILE_EXPORTER_HPP
#define VTK_MESH_FILE_EXPORTER_HPP

#include "FAST/ProcessObject.hpp"

namespace fast {

class VTKMeshFileExporter : public ProcessObject {
    FAST_OBJECT(VTKMeshFileExporter);
    public:
        void setFilename(std::string filename);
    private:
        VTKMeshFileExporter();
        void execute();

        std::string mFilename;
};

}

#endif
