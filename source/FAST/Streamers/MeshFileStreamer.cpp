#include <FAST/Importers/VTKMeshFileImporter.hpp>
#include <FAST/Data/Mesh.hpp>
#include "MeshFileStreamer.hpp"

namespace fast {

MeshFileStreamer::MeshFileStreamer() {
    createOutputPort<Mesh>(0);
}

DataObject::pointer MeshFileStreamer::getDataFrame(std::string filename) {
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(filename);
    importer->setMainDevice(getMainDevice());
    DataPort::pointer port = importer->getOutputPort();
    importer->update(0);
    return port->getNextFrame();
}

}
