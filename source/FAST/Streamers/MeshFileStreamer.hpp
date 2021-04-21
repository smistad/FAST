#pragma once

#include "FAST/Streamers/FileStreamer.hpp"

namespace fast {

/**
 * @brief Stream a sequence of Mesh stored in VTK polydata files
 *
 * This streamer uses the VTKMeshFileImporter
 *
 * <h3>Output ports</h3>
 * - 0: Mesh
 *
 * @ingroup streamers
 */
class FAST_EXPORT  MeshFileStreamer : public FileStreamer {
    FAST_OBJECT(MeshFileStreamer)
    protected:
        DataObject::pointer getDataFrame(std::string filename);
    private:
        MeshFileStreamer();

};

} // end namespace fast
