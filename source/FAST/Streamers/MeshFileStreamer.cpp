#include <FAST/Importers/VTKMeshFileImporter.hpp>
#include <FAST/Data/Mesh.hpp>
#include "MeshFileStreamer.hpp"

namespace fast {

MeshFileStreamer::MeshFileStreamer() {
    createOutputPort<Mesh>(0, OUTPUT_DYNAMIC);
    setMaximumNumberOfFrames(50); // Set default maximum number of frames to 50
}

DataObject::pointer MeshFileStreamer::getDataFrame(std::string filename) {
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(filename);
    importer->setMainDevice(getMainDevice());
    importer->update();
    return importer->getOutputData<Mesh>();
}

}
