#pragma once

#include <FAST/Importers/FileImporter.hpp>

namespace fast {

/**
 * @brief Import NIfTI image files
 *
 * Supports reading both compressed (.nii.gz) and uncompressed (.nii) NIFTI files.
 * Supports version 1 and 2 of the NIFTI format.
 *
 * Outputs:
 * - 0: Image
 *
 * @todo Read orientation information
 *
 * @ingroup importers
 */
class FAST_EXPORT NIFTIImporter : public FileImporter {
    FAST_PROCESS_OBJECT(NIFTIImporter)
    public:
        FAST_CONSTRUCTOR(NIFTIImporter, std::string, filename,)
    protected:
        NIFTIImporter();
        void execute();


};
}