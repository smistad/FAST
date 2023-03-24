#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Data/Image.hpp>
#include <utility>
#include "ImageFileStreamer.hpp"

namespace fast {

ImageFileStreamer::ImageFileStreamer() {
    createOutputPort(0, "Image");
}

ImageFileStreamer::ImageFileStreamer(std::string filename, bool loop, bool useTimestamps, int framerate)  {
    createOutputPort(0, "Image");
    setFilenameFormat(std::move(filename));
    if(loop)
        enableLooping();
    setUseTimestamp(useTimestamps);
    setFramerate(framerate);
}


ImageFileStreamer::ImageFileStreamer(std::vector<std::string> filenames, bool loop, bool useTimestamps, int framerate) {
    createOutputPort(0, "Image");
    setFilenameFormats(std::move(filenames));
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
