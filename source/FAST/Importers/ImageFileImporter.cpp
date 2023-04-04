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
#include <FAST/Algorithms/Color/ColorToGrayscale.hpp>

namespace fast {

void ImageFileImporter::init() {
    createOutputPort(0);
    setMainDevice(Host::getInstance()); // Default is to put image on host
    createBooleanAttribute("grayscale", "Grayscale", "Whether to convert image to grayscale or not", m_grayscale);
}

ImageFileImporter::ImageFileImporter() {
    init();
}

ImageFileImporter::ImageFileImporter(std::string filename, bool grayscale) : FileImporter(filename) {
    init();
    setGrayscale(grayscale);
}

inline bool matchExtension(std::string filename, std::string extension) {
    // Convert to lower case first
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return filename.substr(filename.size() - extension.size()) == extension;

}

void ImageFileImporter::execute() {
    if(m_filename.empty())
        throw Exception("No filename was given to the ImageFileImporter");

    if(!fileExists(m_filename))
        throw FileNotFoundException(m_filename);

    // Get file extension
    size_t pos = m_filename.rfind(".", -5);
    Image::pointer data;
    if(pos == std::string::npos) {
        reportWarning() << "Filename " << m_filename << " had no extension, guessing it to be DICOM.." << reportEnd();
#ifdef FAST_MODULE_DICOM
        auto importer = DICOMFileImporter::New();
        importer->setMainDevice(getMainDevice());
        importer->setFilename(m_filename);
        DataChannel::pointer port = importer->getOutputPort();
        importer->update(); // Have to to update because otherwise the data will not be available
        data = port->getNextFrame<Image>();
#else
        throw Exception("The ImageFileImporter needs the dicom module (DCMTK) to be enabled in order to read dicom files.");
#endif
    } else {
        if(matchExtension(m_filename, "mhd")) {
            MetaImageImporter::pointer importer = MetaImageImporter::New();
            importer->setMainDevice(getMainDevice());
            importer->setFilename(m_filename);
            DataChannel::pointer port = importer->getOutputPort();
            importer->update(); // Have to to update because otherwise the data will not be available
            data = port->getNextFrame<Image>();
        } else if(matchExtension(m_filename, "dcm")) {
#ifdef FAST_MODULE_DICOM
            auto importer = DICOMFileImporter::New();
            importer->setFilename(m_filename);
            importer->setMainDevice(getMainDevice());
            DataChannel::pointer port = importer->getOutputPort();
            importer->update(); // Have to to update because otherwise the data will not be available
            data = port->getNextFrame<Image>();
#else
            throw Exception("The ImageFileImporter needs the dicom module (DCMTK) to be enabled in order to read dicom files.");
#endif
        } else if(matchExtension(m_filename, "jpg") ||
                  matchExtension(m_filename, "jpeg") ||
                  matchExtension(m_filename, "png") ||
                  matchExtension(m_filename, "bmp") ||
                  matchExtension(m_filename, "tif") ||
                  matchExtension(m_filename, "tiff")
                  ) {
#ifdef FAST_MODULE_VISUALIZATION
            auto importer = ImageImporter::New();
            importer->setFilename(m_filename);
            importer->setGrayscale(false);
            importer->setMainDevice(getMainDevice());
            DataChannel::pointer port = importer->getOutputPort();
            importer->update(); // Have to to update because otherwise the data will not be available
            data = port->getNextFrame<Image>();
#else
            throw Exception("Importing regular images requires FAST built with Qt");
#endif
        } else if(matchExtension(m_filename, "nii") ||
                matchExtension(m_filename, "nii.gz")) {
            auto importer = NIFTIImporter::create(m_filename);
            importer->setMainDevice(getMainDevice());
            data = importer->runAndGetOutputData<Image>();
        } else {
            throw Exception("The ImageFileImporter does not recognize the file extension of the filename " + m_filename);
        }
    }

    if(m_grayscale && data->getNrOfChannels() > 1) {
        data = ColorToGrayscale::create()->connect(data)->runAndGetOutputData<Image>();
    }

    addOutputData(0, data);
}

void ImageFileImporter::loadAttributes() {
    setFilename(getStringAttribute("filename"));
    setGrayscale(getBooleanAttribute("grayscale"));
}

void ImageFileImporter::setGrayscale(bool grayscale) {
    m_grayscale = grayscale;
}

}
