#pragma once

#include <FAST/Importers/FileImporter.hpp>
#include <string>

namespace fast {

/**
 * @brief Read DICOM image data (both 2D and 3D).
 *
 * This importer uses the DCMTK library to load DICOM image data from disk.
 *
 * @ingroup importers
 */
class FAST_EXPORT DICOMFileImporter : public FileImporter {
    FAST_PROCESS_OBJECT(DICOMFileImporter)
    public:
        FAST_CONSTRUCTOR(DICOMFileImporter,
                         std::string, filename,,
                         bool, loadSeries, = true
        )
        void setLoadSeries(bool load);
    private:
        DICOMFileImporter();
        void execute() override;

        bool mLoadSeries = true;
};

}
