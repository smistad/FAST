#ifndef FAST_MESH_FILE_STREAMER_HPP_
#define FAST_MESH_FILE_STREAMER_HPP_

#include "FAST/Streamers/FileStreamer.hpp"

namespace fast {

class FAST_EXPORT  MeshFileStreamer : public FileStreamer {
    FAST_OBJECT(MeshFileStreamer)
    protected:
        DataObject::pointer getDataFrame(std::string filename);
    private:
        MeshFileStreamer();

};

} // end namespace fast

#endif
