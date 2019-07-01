#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Data/Image.hpp>
#include "ImageFileStreamer.hpp"

namespace fast {

ImageFileStreamer::ImageFileStreamer() {
    createOutputPort<Image>(0);
}

DataObject::pointer ImageFileStreamer::getDataFrame(std::string filename) {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(filename);
    importer->setMainDevice(getMainDevice());
    DataPort::pointer port = importer->getOutputPort();
    importer->update();
    return port->getNextFrame();
}

}
