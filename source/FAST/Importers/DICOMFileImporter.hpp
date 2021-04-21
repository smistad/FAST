#pragma once

#include "Importer.hpp"
#include <string>

namespace fast {

/**
 * @brief Read DICOM image data (both 2D and 3D).
 *
 * This importer uses the DCMTK library to load DICOM image data from disk.
 *
 * @ingroup importers
 */
class FAST_EXPORT DICOMFileImporter : public Importer {
    FAST_OBJECT(DICOMFileImporter)
    public:
        void setFilename(std::string filename);
        void setLoadSeries(bool load);
    private:
        DICOMFileImporter();
        void execute() override;

        bool mLoadSeries = true;
        std::string mFilename = "";
};

}
