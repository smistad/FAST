#ifndef VTK_SURFACE_FILE_EXPORTER_HPP
#define VTK_SURFACE_FILE_EXPORTER_HPP

#include "ProcessObject.hpp"
#include "Surface.hpp"

namespace fast {

class VTKSurfaceFileExporter : public ProcessObject {
    FAST_OBJECT(VTKSurfaceFileExporter);
    public:
        void setInput(MeshData::pointer input);
        void setFilename(std::string filename);
    private:
        VTKSurfaceFileExporter();
        void execute();

        std::string mFilename;
        MeshData::pointer mInput;

};

}

#endif
