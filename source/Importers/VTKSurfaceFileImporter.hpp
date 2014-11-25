#ifndef VTK_SURFACE_FILE_IMPORTER_HPP
#define VTK_SURFACE_FILE_IMPORTER_HPP

#include "Importer.hpp"
#include "Surface.hpp"
#include <string>

namespace fast {

class VTKSurfaceFileImporter : public Importer {
    FAST_OBJECT(VTKSurfaceFileImporter)
    public:
        void setFilename(std::string filename);
        Mesh::pointer getOutput();
    private:
        VTKSurfaceFileImporter();
        void execute();

        std::string mFilename;
        Mesh::pointer mOutput;
};

} // end namespace fast

#endif
