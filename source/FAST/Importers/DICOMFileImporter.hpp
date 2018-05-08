#ifndef FAST_DICOM_FILE_IMPORTER_HPP_
#define FAST_DICOM_FILE_IMPORTER_HPP_

#include "Importer.hpp"
#include <string>

namespace fast {

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

#endif
