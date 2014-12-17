#ifndef VTK_SURFACE_FILE_IMPORTER_HPP
#define VTK_SURFACE_FILE_IMPORTER_HPP

#include "Importer.hpp"
#include <string>
#include "Mesh.hpp"

namespace fast {

class VTKMeshFileImporter : public Importer {
    FAST_OBJECT(VTKMeshFileImporter)
    public:
        void setFilename(std::string filename);
        Mesh::pointer getOutput();
    private:
        VTKMeshFileImporter();
        void execute();

        std::string mFilename;
        Mesh::pointer mOutput;
};

} // end namespace fast

#endif
