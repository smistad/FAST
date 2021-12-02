#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Data/Image.hpp>
#include "ImageFileStreamer.hpp"

namespace fast {

ImageFileStreamer::ImageFileStreamer() {
    createOutputPort(0, "Image");
}

ImageFileStreamer::ImageFileStreamer(std::string filename, bool loop, bool useTimestamps, int framerate) {
    createOutputPort(0, "Image");
    setFilenameFormat(filename);
    if(loop)
        enableLooping();
    setUseTimestamp(useTimestamps);
    setFramerate(framerate);
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
