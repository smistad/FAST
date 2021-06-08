#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Data/Image.hpp>
#include "ImageFileStreamer.hpp"

namespace fast {

ImageFileStreamer::ImageFileStreamer() {
    createOutputPort<Image>(0);
}

ImageFileStreamer::ImageFileStreamer(std::string filename) {
    createOutputPort(0, "Image");
    setFilenameFormat(filename);
}

DataObject::pointer ImageFileStreamer::getDataFrame(std::string filename) {
    auto importer = ImageFileImporter::New();
    importer->setFilename(filename);
    importer->setMainDevice(getMainDevice());
    auto port = importer->getOutputPort();
    importer->update();
    return port->getNextFrame();
}

}
