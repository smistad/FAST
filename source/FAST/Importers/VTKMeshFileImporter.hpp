#ifndef VTK_SURFACE_FILE_IMPORTER_HPP
#define VTK_SURFACE_FILE_IMPORTER_HPP

#include "Importer.hpp"
#include <string>

namespace fast {

class VTKMeshFileImporter : public Importer {
    FAST_OBJECT(VTKMeshFileImporter)
    public:
        void setFilename(std::string filename);
    private:
        VTKMeshFileImporter();
        void execute();

        std::string mFilename;
};

} // end namespace fast

#endif
