#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Data/Image.hpp>
#include "ImageFileStreamer.hpp"

namespace fast {

ImageFileStreamer::ImageFileStreamer() {
    createOutputPort<Image>(0, OUTPUT_DYNAMIC);
    setMaximumNumberOfFrames(50); // Set default maximum number of frames to 50
}

DataObject::pointer ImageFileStreamer::getDataFrame(std::string filename) {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(filename);
    importer->setMainDevice(getMainDevice());
    importer->update();
    return importer->getOutputData<Image>();
}

}
