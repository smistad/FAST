#include <FAST/Importers/VTKMeshFileImporter.hpp>
#include <FAST/Data/Mesh.hpp>
#include "MeshFileStreamer.hpp"

namespace fast {

MeshFileStreamer::MeshFileStreamer() {
    createOutputPort<Mesh>(0);
}
MeshFileStreamer::MeshFileStreamer(std::string filenameFormat, bool loop) {
    createOutputPort<Mesh>(0);
    setFilenameFormat(filenameFormat);
    if(loop)
        enableLooping();
}

DataObject::pointer MeshFileStreamer::getDataFrame(std::string filename) {
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(filename);
    importer->setMainDevice(getMainDevice());
    DataChannel::pointer port = importer->getOutputPort();
    importer->update();
    return port->getNextFrame();
}

}
