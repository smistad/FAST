#ifndef VTK_SURFACE_FILE_EXPORTER_HPP
#define VTK_SURFACE_FILE_EXPORTER_HPP

#include "FAST/ProcessObject.hpp"

namespace fast {

class VTKSurfaceFileExporter : public ProcessObject {
    FAST_OBJECT(VTKSurfaceFileExporter);
    public:
        void setFilename(std::string filename);
    private:
        VTKSurfaceFileExporter();
        void execute();

        std::string mFilename;
};

}

#endif
