#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Data/Image.hpp>
#include <utility>
#include "ImageFileStreamer.hpp"

namespace fast {

ImageFileStreamer::ImageFileStreamer() : FileStreamer() {
    createOutputPort(0, "Image");
    createBooleanAttribute("grayscale", "Convert to grayscale", "Convert image to grayscale if source image is in color", m_grayscale);
}

ImageFileStreamer::ImageFileStreamer(std::string filename, bool loop, bool useTimestamps, int framerate, bool grayscale) : ImageFileStreamer()  {
    setFilenameFormat(std::move(filename));
    if(loop)
        enableLooping();
    setUseTimestamp(useTimestamps);
    setFramerate(framerate);
    setGrayscale(grayscale);
}


ImageFileStreamer::ImageFileStreamer(std::vector<std::string> filenames, bool loop, bool useTimestamps, int framerate, bool grayscale) : ImageFileStreamer() {
    createOutputPort(0, "Image");
    setFilenameFormats(std::move(filenames));
    if(loop)
        enableLooping();
    setUseTimestamp(useTimestamps);
    setFramerate(framerate);
    setGrayscale(grayscale);
}

DataObject::pointer ImageFileStreamer::getDataFrame(std::string filename) {
    auto importer = ImageFileImporter::create(filename, m_grayscale);
    importer->setFilename(filename);
    importer->setMainDevice(getMainDevice());
    auto port = importer->getOutputPort();
    importer->update();
    return port->getNextFrame();
}

void ImageFileStreamer::setGrayscale(bool grayscale) {
    m_grayscale = grayscale;
}

void ImageFileStreamer::loadAttributes() {
    FileStreamer::loadAttributes();
    setGrayscale(getBooleanAttribute("grayscale"));
}

}
