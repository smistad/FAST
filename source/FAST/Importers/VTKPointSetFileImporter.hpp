#ifndef VTK_POINT_SET_FILE_IMPORTER_HPP
#define VTK_POINT_SET_FILE_IMPORTER_HPP

#include "Importer.hpp"
#include <string>

namespace fast {

class VTKPointSetFileImporter : public Importer {
    FAST_OBJECT(VTKPointSetFileImporter)
    public:
        void setFilename(std::string filename);
    private:
        VTKPointSetFileImporter();
        void execute();

        std::string mFilename;
};

} // end namespace fast

#endif
