#ifndef VTK_POINT_SET_FILE_IMPORTER_HPP
#define VTK_POINT_SET_FILE_IMPORTER_HPP

#include "Importer.hpp"
#include "PointSet.hpp"
#include <string>

namespace fast {

class VTKPointSetFileImporter : public Importer {
    FAST_OBJECT(VTKPointSetFileImporter)
    public:
        void setFilename(std::string filename);
        PointSet::pointer getOutput();
    private:
        VTKPointSetFileImporter();
        void execute();

        std::string mFilename;
};

} // end namespace fast

#endif
