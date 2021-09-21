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
    FAST_PROCESS_OBJECT(MeshFileStreamer)
    public:
        /**
         * @brief Create instance
         * @param filenameFormat String of path and format of images. E.g. /path/to/files/frame_#.mhd. The hash sign #
         * will be replaced by an index
         * @param loop Whether to loop the recording or not
         * @return instance
         */
        FAST_CONSTRUCTOR(MeshFileStreamer,
                         std::string, filenameFormat,,
                         bool, loop, = false
        );
    protected:
        DataObject::pointer getDataFrame(std::string filename);
        MeshFileStreamer();

};

} // end namespace fast
