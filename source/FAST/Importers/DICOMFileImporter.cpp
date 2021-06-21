#include "DICOMFileImporter.hpp"
#include <dcmtk/ofstd/ofstd.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include "FAST/Data/Image.hpp"

namespace fast {

DICOMFileImporter::DICOMFileImporter() {
    createOutputPort<Image>(0);
}

DICOMFileImporter::DICOMFileImporter(std::string filename, bool loadSeries) : FileImporter(filename) {
    createOutputPort<Image>(0);
    setLoadSeries(loadSeries);
}

void DICOMFileImporter::setLoadSeries(bool load) {
    mLoadSeries = load;
    mIsModified = true;
}

template <class T>
static void* readRawData(const DiPixel* pixelData) {
    const void* data = pixelData->getData();
    T* outputData = new T[pixelData->getCount()];
    memcpy(outputData, data, pixelData->getCount()*sizeof(T));
    return outputData;
}

static DataType getDataType(const DicomImage &image) {
    const DiPixel* pixelData = image.getInterData();
    EP_Representation rep = pixelData->getRepresentation();
    DataType type;
    switch(rep) {
        case EPR_Sint8:
            type = TYPE_INT8;
            break;
        case EPR_Uint8:
            type = TYPE_UINT8;
            break;
        case EPR_Sint16:
            type = TYPE_INT16;
            break;
        case EPR_Uint16:
            type = TYPE_UINT16;
            break;
        case EPR_Sint32:
        case EPR_Uint32:
            throw Exception("Unsupported data type of 32 bit integer in DICOMFileImporter");
            break;
    }
    return type;
}

static void* getDataFromImage(const DicomImage &image, DataType &type) {
    const DiPixel* pixelData = image.getInterData();
    EP_Representation rep = pixelData->getRepresentation();
    void* data;
    switch(rep) {
        case EPR_Sint8:
            type = TYPE_INT8;
            data = readRawData<char>(pixelData);
            break;
        case EPR_Uint8:
            type = TYPE_UINT8;
            data = readRawData<uchar>(pixelData);
            break;
        case EPR_Sint16:
            type = TYPE_INT16;
            data = readRawData<short>(pixelData);
            break;
        case EPR_Uint16:
            type = TYPE_UINT16;
            data = readRawData<ushort>(pixelData);
            break;
        case EPR_Sint32:
        case EPR_Uint32:
            throw Exception("Unsupported data type of 32 bit integer in DICOMFileImporter");
            break;
    }

    return data;
}

static void* inserSliceFromImage(const DicomImage &image, int width, int height, int sliceNr, void* data) {
    const DiPixel* pixelData = image.getInterData();
    EP_Representation rep = pixelData->getRepresentation();
    int currentPosition = width*height*sliceNr;
    DataType type;
    void* sliceData;
    switch(rep) {
        case EPR_Sint8:
            type = TYPE_INT8;
            sliceData = readRawData<char>(pixelData);
            memcpy((char*)data + currentPosition, sliceData, width*height*getSizeOfDataType(type, 1));
            break;
        case EPR_Uint8:
            type = TYPE_UINT8;
            sliceData = readRawData<uchar>(pixelData);
            memcpy((uchar*)data + currentPosition, sliceData, width*height*getSizeOfDataType(type, 1));
            break;
        case EPR_Sint16:
            type = TYPE_INT16;
            sliceData = readRawData<short>(pixelData);
            memcpy((short*)data + currentPosition, sliceData, width*height*getSizeOfDataType(type, 1));
            break;
        case EPR_Uint16:
            type = TYPE_UINT16;
            sliceData = readRawData<ushort>(pixelData);
            memcpy((ushort*)data + currentPosition, sliceData, width*height*getSizeOfDataType(type, 1));
            break;
        case EPR_Sint32:
        case EPR_Uint32:
            throw Exception("Unsupported data type of 32 bit integer in DICOMFileImporter");
            break;
    }
    deleteArray(sliceData, type);

    return data;
}

void DICOMFileImporter::execute() {
    if(m_filename == "")
        throw Exception("DICOMFileImporter needs filename to be set");

    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(m_filename.c_str());
    if(status.good()) {
        // Get pixel spacing
        Float64 spacingX;
        Float64 spacingY;
        Float64 spacingZ;
        fileformat.getDataset()->findAndGetFloat64(DCM_PixelSpacing, spacingX, 0);
        fileformat.getDataset()->findAndGetFloat64(DCM_PixelSpacing, spacingY, 1);
        fileformat.getDataset()->findAndGetFloat64(DCM_SliceThickness, spacingZ);
        if(mLoadSeries) {
            // Get series ID first
            OFString seriesID;
            if(!fileformat.getDataset()->findAndGetOFString(DCM_SeriesInstanceUID, seriesID).good())
                throw Exception("Could not get series instance UID of DICOM file.");

            // Get all files in directory which has same series instance UID
            std::string dirName = getDirName(m_filename);
            std::vector<std::string> files = getDirectoryList(dirName);
            std::vector<std::string> seriesFiles;
            for(auto&& file : files) {
                OFString seriesID2;
                DcmFileFormat fileformat2;
                OFCondition status = fileformat2.loadFile((dirName + "/" + file).c_str());
                fileformat2.getDataset()->findAndGetOFString(DCM_SeriesInstanceUID, seriesID2);
                if(seriesID == seriesID2) {
                    seriesFiles.push_back(dirName + "/" + file);
                }
            }

            // Get size and type of image
            DicomImage image(m_filename.c_str());
            const int width = image.getWidth();
            const int height = image.getHeight();
            const int depth = seriesFiles.size();
            const DataType type = getDataType(image);

			std::cout << "DEPTH: " << depth << std::endl;
            // Allocate space for volume
            void* data = allocateDataArray(width*height*depth, type, 1);

            // Get data of each slice
            for(auto&& file : seriesFiles) {
                DcmFileFormat fileformat2;
                OFCondition status = fileformat2.loadFile(file.c_str());
                Sint32 sliceNr;
                fileformat2.getDataset()->findAndGetSint32(DCM_InstanceNumber, sliceNr);
                inserSliceFromImage(DicomImage(file.c_str()), width, height, sliceNr-1, data);
            }

            auto output = Image::create(image.getWidth(), image.getHeight(), seriesFiles.size(), type, 1, data);
            output->setSpacing(spacingX, spacingY, spacingZ);
            deleteArray(data, type);
            addOutputData(0, output);
        } else {
            DicomImage image(m_filename.c_str());
            DataType type;
            void* data = getDataFromImage(image, type);

            auto output = Image::create(image.getWidth(), image.getHeight(), type, 1, data);
            output->setSpacing(spacingX, spacingY, 1);
            deleteArray(data, type);
            addOutputData(0, output);
        }
    } else {
        throw Exception("Error: cannot read DICOM file " + m_filename + "(" + std::string(status.text()) + ")");
    }
}

}