#include "ImageFileImporter.hpp"
#include "MetaImageImporter.hpp"
#ifdef FAST_MODULE_DICOM
#include "DICOMFileImporter.hpp"
#endif
#ifdef FAST_MODULE_VISUALIZATION
#include "ImageImporter.hpp"
#endif
#include "FAST/Data/Image.hpp"
#include "NIFTIImporter.hpp"
#include <algorithm>
#include <utility>

namespace fast {

ImageFileImporter::ImageFileImporter() {
    createOutputPort<Image>(0);
    setMainDevice(Host::getInstance()); // Default is to put image on host
}

ImageFileImporter::ImageFileImporter(std::string filename) : FileImporter(filename) {
    createOutputPort<Image>(0);
    setMainDevice(Host::getInstance()); // Default is to put image on host
    setFilename(filename);
}

inline bool matchExtension(std::string extension, std::string extension2) {
    // Convert to lower case first
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    std::transform(extension2.begin(), extension2.end(), extension2.begin(), ::tolower);
    return extension == extension2;

}

void ImageFileImporter::execute() {
    if(m_filename.empty())
        throw Exception("No filename was given to the ImageFileImporter");

    if(!fileExists(m_filename))
        throw FileNotFoundException(m_filename);

    // Get file extension
    size_t pos = m_filename.rfind(".", -5);
    if(pos == std::string::npos) {
        reportWarning() << "Filename " << m_filename << " had no extension, guessing it to be DICOM.." << reportEnd();
#ifdef FAST_MODULE_DICOM
        auto importer = DICOMFileImporter::New();
        importer->setMainDevice(getMainDevice());
        importer->setFilename(m_filename);
        DataChannel::pointer port = importer->getOutputPort();
        importer->update(); // Have to to update because otherwise the data will not be available
        Image::pointer data = port->getNextFrame<Image>();
        addOutputData(0, data);
#else
        throw Exception("The ImageFileImporter needs the dicom module (DCMTK) to be enabled in order to read dicom files.");
#endif
    } else {
        std::string ext = m_filename.substr(pos + 1);
        if(matchExtension(ext, "mhd")) {
            MetaImageImporter::pointer importer = MetaImageImporter::New();
            importer->setMainDevice(getMainDevice());
            importer->setFilename(m_filename);
            DataChannel::pointer port = importer->getOutputPort();
            importer->update(); // Have to to update because otherwise the data will not be available
            Image::pointer data = port->getNextFrame<Image>();
            addOutputData(0, data);
        } else if(matchExtension(ext, "dcm")) {
#ifdef FAST_MODULE_DICOM
            auto importer = DICOMFileImporter::New();
            importer->setFilename(m_filename);
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
                  matchExtension(ext, "bmp") ||
                  matchExtension(ext, "tif") ||
                  matchExtension(ext, "tiff")
                  ) {
#ifdef FAST_MODULE_VISUALIZATION
            auto importer = ImageImporter::New();
            importer->setFilename(m_filename);
            importer->setMainDevice(getMainDevice());
            DataChannel::pointer port = importer->getOutputPort();
            importer->update(); // Have to to update because otherwise the data will not be available
            Image::pointer data = port->getNextFrame<Image>();
            addOutputData(0, data);
#else
            throw Exception("Importing regular images requires FAST built with Qt");
#endif
        } else if(matchExtension(ext, "nii") ||
                matchExtension(ext, "nii.gz")) {
            auto importer = NIFTIImporter::create(m_filename);
            importer->setMainDevice(getMainDevice());
            addOutputData(0, importer->runAndGetOutputData<Image>());
        } else {
            throw Exception("The ImageFileImporter does not recognize the file extension " + ext);
        }
    }

}

void ImageFileImporter::loadAttributes() {
    setFilename(getStringAttribute("filename"));
}

}
