#include "ImageFileImporter.hpp"
#include "MetaImageImporter.hpp"
#ifdef FAST_MODULE_DICOM
#include "DICOMFileImporter.hpp"
#endif
#ifdef FAST_MODULE_VISUALIZATION
#include "ImageImporter.hpp"
#endif
#include "FAST/Data/Image.hpp"
#include <algorithm>

namespace fast {

void ImageFileImporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

ImageFileImporter::ImageFileImporter() {
    mFilename = "";
    createOutputPort<Image>(0);
    setMainDevice(Host::getInstance()); // Default is to put image on host
}

inline bool matchExtension(std::string extension, std::string extension2) {
    // Convert to lower case first
    std::transform(extension2.begin(), extension2.end(), extension2.begin(), ::tolower);
    return extension == extension2;

}

void ImageFileImporter::execute() {
    if(mFilename == "")
        throw Exception("No filename was given to the ImageFileImporter");

    if(!fileExists(mFilename))
        throw FileNotFoundException(mFilename);

    // Get file extension
    size_t pos = mFilename.rfind(".", -5);
    if(pos == std::string::npos) {
        reportWarning() << "Filename " << mFilename << " had no extension, guessing it to be DICOM.." << reportEnd();
#ifdef FAST_MODULE_DICOM
        auto importer = DICOMFileImporter::New();
        importer->setMainDevice(getMainDevice());
        importer->setFilename(mFilename);
        DataChannel::pointer port = importer->getOutputPort();
        importer->update(); // Have to to update because otherwise the data will not be available
        Image::pointer data = port->getNextFrame<Image>();
        addOutputData(0, data);
#else
        throw Exception("The ImageFileImporter needs the dicom module (DCMTK) to be enabled in order to read dicom files.");
#endif
    } else {
        std::string ext = mFilename.substr(pos + 1);
        if(matchExtension(ext, "mhd")) {
            MetaImageImporter::pointer importer = MetaImageImporter::New();
            importer->setMainDevice(getMainDevice());
            importer->setFilename(mFilename);
            DataChannel::pointer port = importer->getOutputPort();
            importer->update(); // Have to to update because otherwise the data will not be available
            Image::pointer data = port->getNextFrame<Image>();
            addOutputData(0, data);
        } else if(matchExtension(ext, "dcm")) {
#ifdef FAST_MODULE_DICOM
            auto importer = DICOMFileImporter::New();
            importer->setFilename(mFilename);
            importer->setMainDevice(getMainDevice());
            DataChannel::pointer port = importer->getOutputPort();
            importer->update(); // Have to to update because otherwise the data will not be available
            Image::pointer data = port->getNextFrame<Image>();
            addOutputData(0, data);
#else
            throw Exception("The ImageFileImporter needs the dicom module (DCMTK) to be enabled in order to read dicom files.");
#endif
        } else if(matchExtension(ext, "jpg") ||
                  matchExtension(ext, "jpeg") ||
                  matchExtension(ext, "png") ||
                  matchExtension(ext, "bmp")) {
#ifdef FAST_MODULE_VISUALIZATION
            auto importer = ImageImporter::New();
            importer->setFilename(mFilename);
            importer->setMainDevice(getMainDevice());
            DataChannel::pointer port = importer->getOutputPort();
            importer->update(); // Have to to update because otherwise the data will not be available
            Image::pointer data = port->getNextFrame<Image>();
            addOutputData(0, data);
#else
            throw Exception("The ImageFileImporter needs the visualization module (Qt) to be enabled in order to read image files.");
#endif
        } else {
            throw Exception("The ImageFileImporter does not recognize the file extension " + ext);
        }
    }

}

}
