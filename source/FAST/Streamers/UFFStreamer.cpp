#include "UFFStreamer.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Reporter.hpp>

#define H5_BUILT_AS_DYNAMIC_LIB
#include <H5Cpp.h>
#include <FAST/Algorithms/Ultrasound/ScanConverter.hpp>
#include <FAST/Algorithms/Ultrasound/EnvelopeAndLogCompressor.hpp>

namespace fast {


static void pol2cart(float r, float th, float &x, float &y) {
    x = r * std::cos(th);
    y = r * std::sin(th);
}

/**
* A data structure for the ultrasound file format (UFF).
* Stores relevant parameters.
*/
class UFFData {
public:
    Vector3f spacing;
    std::string groupName;
    std::string x_axis_name, y_axis_name;
    uint width;
    uint height;
    bool isScanConverted;
    std::string dataGroupName;
    int numFrames;
    bool polarCoordinates;
    std::vector<Image::pointer> dataScanconverted;
    std::vector<Image::pointer> iqData;

    std::vector<float> azimuth_axis;
    std::vector<float> depth_axis;

    bool hasGrayscaleData() {
        return !dataScanconverted.empty();
    }
};

class UFFReader {
    public:
        UFFReader();
        void open(std::string filename);
        void close();
        std::string findHDF5BeamformedDataGroupName();
        std::shared_ptr<UFFData> getUFFData();

    private:
        H5::H5File mFile;
        void getAxisNames(H5::Group scanGroup, std::shared_ptr<UFFData> dataStruct);
        void getImageSize(H5::Group scanGroup, std::shared_ptr<UFFData> dataStruct);
        void getSpacing(H5::Group scanGroup, std::shared_ptr<UFFData> dataStruct);
        H5::Group getDataGroupAndIsScanconverted(std::shared_ptr<UFFData> dataStruct);
        void readData(H5::Group dataGroup, std::shared_ptr<UFFData> dataStruct);
        void readNotScanconvertedData(H5::Group dataGroup, std::shared_ptr<UFFData> dataStruct);
        void readScanconvertedData(H5::Group dataGroup, std::shared_ptr<UFFData> dataStruct);
};

//Operator function to be used with H5Literate
herr_t file_info(hid_t loc_id, const char* name, const H5L_info_t* linfo, void* opdata) {
    auto groupNames = (std::vector<std::string>*)opdata;
    groupNames->push_back(name);

    return 0;
}

static std::string readStringAttribute(const H5::Attribute& att) {
    std::string result;
    att.read(att.getDataType(), result);
    return result;
}

UFFReader::UFFReader() {
}

void UFFReader::open(std::string filename) {
    mFile = H5::H5File(filename.c_str(), H5F_ACC_RDONLY);
}

void UFFReader::close() {
    mFile.close();
}

std::string UFFReader::findHDF5BeamformedDataGroupName() {
    std::string selectedGroupName;
    // Find HDF5 group name auomatically
    std::vector<std::string> groupNames;
    std::vector<std::string> beamformedDataGroups;
    herr_t idx = H5Literate(mFile.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, file_info, &groupNames);
    for (auto groupName : groupNames) {
        auto group = mFile.openGroup(groupName);
        auto classAttribute = group.openAttribute("class");
        auto className = readStringAttribute(classAttribute);
        if (className == "uff.beamformed_data")
            beamformedDataGroups.push_back(groupName);
    }
    if (beamformedDataGroups.empty()) {
        throw Exception("No beamformed_data class group found");
    }
    selectedGroupName = beamformedDataGroups[0];
    return selectedGroupName;
}

std::shared_ptr<UFFData> UFFReader::getUFFData() {
    auto retVal = std::make_shared<UFFData>();
    retVal->groupName = findHDF5BeamformedDataGroupName();

    H5::Group scanGroup = mFile.openGroup(retVal->groupName + "/scan");
    //NB: Function order matters
    getAxisNames(scanGroup, retVal);
    getImageSize(scanGroup, retVal);

    H5::Group dataGroup = getDataGroupAndIsScanconverted(retVal);
    getSpacing(scanGroup, retVal);

    readData(dataGroup, retVal);

    return retVal;
}

void UFFReader::getAxisNames(H5::Group scanGroup, std::shared_ptr<UFFData> dataStruct) {
    auto classAttribute = scanGroup.openAttribute("class");
    auto className = readStringAttribute(classAttribute);

    if (className == "uff.linear_scan") {
        dataStruct->x_axis_name = "x_axis";
        dataStruct->y_axis_name = "z_axis";
        dataStruct->polarCoordinates = false;
    } else {
        dataStruct->x_axis_name = "azimuth_axis";
        dataStruct->y_axis_name = "depth_axis";
        dataStruct->polarCoordinates = true;
    }
    Reporter::info() << "UFF axis names " << dataStruct->x_axis_name << " " << dataStruct->y_axis_name << Reporter::end();
}

void UFFReader::getImageSize(H5::Group scanGroup, std::shared_ptr<UFFData> dataStruct) {
    {
        auto dataset = scanGroup.openDataSet(dataStruct->x_axis_name);
        auto dataspace = dataset.getSpace();
        hsize_t dims_out[2];
        int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
        dataStruct->width = dims_out[1];
    }
    {
        auto dataset = scanGroup.openDataSet(dataStruct->y_axis_name);
        auto dataspace = dataset.getSpace();
        hsize_t dims_out[2];
        int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
        dataStruct->height = dims_out[1];
    }
    Reporter::info() << "UFF Image size was found to be " << dataStruct->width << " " << dataStruct->height << Reporter::end();
}

void UFFReader::getSpacing(H5::Group scanGroup, std::shared_ptr<UFFData> dataStruct) {
    {
        //TODO: Verify that all spacings are equal if dataStruct.spacing is to be used

        //Should we use the "spacing vectors" for scanconverted data as well?
        //Now the data is read into azimuth_axis and depth_axis in both cases, but the
        //variable names don't sense for the scanvonverted case.

        auto dataset = scanGroup.openDataSet(dataStruct->x_axis_name);
        auto x_axis = std::make_unique<float[]>(dataStruct->width);
        dataset.read(x_axis.get(), H5::PredType::NATIVE_FLOAT);
        if(dataStruct->isScanConverted)
            dataStruct->spacing.x() = std::fabs(x_axis[0] - x_axis[1])*1000;
        else
            dataStruct->spacing.x() = std::fabs(x_axis[0] - x_axis[1]); //Don't multiply radians with 1000

        dataStruct->azimuth_axis.resize(dataStruct->width);
        for(int i = 0; i < dataStruct->width; ++i) {
            dataStruct->azimuth_axis[i] = x_axis[i];
        }
    }
    {
        auto dataset = scanGroup.openDataSet(dataStruct->y_axis_name);
        auto z_axis = std::make_unique<float[]>(dataStruct->height);
        dataset.read(z_axis.get(), H5::PredType::NATIVE_FLOAT);

        dataStruct->spacing.y() = std::fabs(z_axis[0] - z_axis[1])*1000;

        dataStruct->depth_axis.resize(dataStruct->height);
        for(int i = 0; i < dataStruct->height; ++i) {
            dataStruct->depth_axis[i] = z_axis[i];
        }
    }
    Reporter::info() << "Spacing in UFF file was " << dataStruct->spacing.transpose() << Reporter::end();
}

H5::Group UFFReader::getDataGroupAndIsScanconverted(std::shared_ptr<UFFData> dataStruct) {
    H5::Group dataGroup;
    dataStruct->dataGroupName = dataStruct->groupName + "/data";
    dataStruct->isScanConverted = false;

    // Save old error handler
    H5E_auto2_t old_func;
    void *old_client_data;
    hid_t error_stack = H5E_DEFAULT;
    H5Eget_auto2( error_stack, &old_func, &old_client_data);

    // Turn off error handling to avoid error output from try - catch
    H5Eset_auto2(error_stack, NULL, NULL);

    try {
        // Is IQ data
        dataGroup = mFile.openGroup(dataStruct->dataGroupName);
    }
    catch (...) {
        // Is grayscale data
        dataStruct->dataGroupName = dataStruct->groupName;
        dataGroup = mFile.openGroup(dataStruct->dataGroupName);
        dataStruct->isScanConverted = true;
    }

    // Restore previous error handler
    H5Eset_auto2(error_stack, old_func, old_client_data);

    Reporter::info() << "UFF Image data " << ((dataStruct->isScanConverted) ? "is" : "is not") << " scanconverted." << Reporter::end();
    return dataGroup;
}

void UFFReader::readData(H5::Group dataGroup, std::shared_ptr<UFFData> dataStruct) {

    if(dataStruct->isScanConverted)
        readScanconvertedData(dataGroup, dataStruct);
    else
        readNotScanconvertedData(dataGroup, dataStruct);
}


void UFFReader::readNotScanconvertedData(H5::Group dataGroup, std::shared_ptr<UFFData> dataStruct) {
    auto dataset = dataGroup.openDataSet("imag");
    auto dataspace = dataset.getSpace();
    hsize_t dims_out[4];
    int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);

    if(ndims != 4 && ndims != 2) {
        throw Exception("Exepected 4 or 2 dimensions in UFF file, got " + std::to_string(ndims));
    }

    int frameCount = dims_out[0];
    Reporter::info() << "Number of frames in UFF file: " << frameCount << Reporter::end();
    dataStruct->numFrames = frameCount;//TODO

    auto imagDataset = dataGroup.openDataSet("imag");
    auto imagDataspace = imagDataset.getSpace();
    auto realDataset = dataGroup.openDataSet("real");
    auto realDataspace = realDataset.getSpace();

    std::vector<hsize_t> count;
    std::vector<hsize_t> blockSize;
    std::vector<hsize_t> offset;
    if(ndims == 4) {
        count = { 1, 1, 1, 1 }; // how many blocks to extract
        blockSize = { 1, 1, 1, hsize_t(dataStruct->width * dataStruct->height) }; // block
        offset = { 0, 0, 0, 0 };   // hyperslab offset in the file
    } else {
        count = { 1, 1 }; // how many blocks to extract
        blockSize = { 1, hsize_t(dataStruct->width * dataStruct->height) }; // block
        offset = { 0, 0 };   // hyperslab offset in the file
    }

    H5::DataSpace memspace(ndims, blockSize.data());

    //dataStruct.dataNotScanconverted.resize(frameCount);//Make room for pointers in vector
    dataStruct->iqData.resize(frameCount);//Make room for pointers in vector

    for(int frameNr = 0; frameNr < frameCount; ++frameNr) {
        //std::cout << "Extracting frame " << frameNr << " in UFF file" << std::endl;
        // Extract 1 frame
        auto imaginary = std::make_unique<float[]>(dataStruct->width * dataStruct->height);
        auto real = std::make_unique<float[]>(dataStruct->width * dataStruct->height);
        offset[0] = frameNr;
        imagDataspace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data(), NULL, blockSize.data());
        imagDataset.read(imaginary.get(), H5::PredType::NATIVE_FLOAT, memspace, imagDataspace);
        realDataspace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data(), NULL, blockSize.data());
        realDataset.read(real.get(), H5::PredType::NATIVE_FLOAT, memspace, realDataspace);

        int dataSize = dataStruct->width * dataStruct->height;
        auto complex_image = make_uninitialized_unique<float[]>(dataSize*2);

        for(int y = 0; y < dataStruct->height; ++y) {
            for(int x = 0; x < dataStruct->width; ++x) {
                int pos = x + y * dataStruct->width;
                int pos2 = y + x * dataStruct->height;
                complex_image[pos*2] = real[pos2];
                complex_image[pos*2+1] = imaginary[pos2];
            }
        }
        auto image = Image::create(dataStruct->width, dataStruct->height, TYPE_FLOAT, 2, std::move(complex_image));
        if(frameNr == frameCount-1)
            image->setLastFrame("UFFStreamer");
        dataStruct->iqData[frameNr] = image;
    }//for
}

void UFFReader::readScanconvertedData(H5::Group dataGroup, std::shared_ptr<UFFData> dataStruct) {
    auto dataset = dataGroup.openDataSet("data");
    auto dataspace = dataset.getSpace();
    hsize_t dims_out[4];
    int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);

    if(ndims != 4 && ndims != 2) {
        throw Exception("Exepected 4 or 2 dimensions in UFF file, got " + std::to_string(ndims));
    }

    int frameCount = dims_out[0];
    Reporter::info() << "Number of frames in UFF file: " << frameCount << Reporter::end();

    dataStruct->numFrames = frameCount;

    std::vector<hsize_t> count;
    std::vector<hsize_t> blockSize;
    std::vector<hsize_t> offset;
    if(ndims == 4) {
        count = { 1, 1, 1, 1 }; // how many blocks to extract
        blockSize = { 1, 1, 1, hsize_t(dataStruct->width * dataStruct->height) }; // block
        offset = { 0, 0, 0, 0 };   // hyperslab offset in the file
    } else {
        count = { 1, 1 }; // how many blocks to extract
        blockSize = { 1, hsize_t(dataStruct->width * dataStruct->height) }; // block
        offset = { 0, 0 };   // hyperslab offset in the file
    }

    H5::DataSpace memspace(4, blockSize.data());

    dataStruct->dataScanconverted.resize(frameCount);//Make room for pointers in vector

    for (int frameNr = 0; frameNr < frameCount; ++frameNr) {
        //std::cout << "Extracting frame " << frameNr << " in UFF file" << std::endl;
        // Extract 1 frame
        auto data = std::make_unique<unsigned char[]>(dataStruct->width * dataStruct->height);
        offset[0] = frameNr;
        dataspace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data(), NULL, blockSize.data());
        dataset.read(data.get(), H5::PredType::NATIVE_UCHAR, memspace, dataspace);

        int dataSize = dataStruct->width * dataStruct->height;
        auto image_data = make_uninitialized_unique<uchar[]>(dataSize);
        for (int y = 0; y < dataStruct->height; ++y) {
            for (int x = 0; x < dataStruct->width; ++x) {
                //TODO: Should axes be swapped?
                int pos = y + x * dataStruct->height;
                image_data[x + y * dataStruct->width] = data[pos];
            }
        }
        auto image = Image::create(dataStruct->width, dataStruct->height, TYPE_UINT8, 1, std::move(image_data));
        image->setSpacing(dataStruct->spacing.x(), dataStruct->spacing.y(), dataStruct->spacing.z());
        if(frameNr == frameCount-1)
            image->setLastFrame("UFFStreamer");
        dataStruct->dataScanconverted[frameNr] = image;
    }//for
}

void UFFStreamer::load() {
    if(m_uffData)
        return;
    if(m_filename.empty())
        throw Exception("You must set filename in UFFImageImporter with setFilename()");
    if(!fileExists(m_filename))
        throw FileNotFoundException(m_filename);

    auto uffReader = UFFReader();
    uffReader.open(m_filename);
    m_uffData = uffReader.getUFFData();
}

void UFFStreamer::execute() {
    if(!m_streamIsStarted) {
        load();
        m_streamIsStarted = true;
        m_thread = std::make_unique<std::thread>(std::bind(&UFFStreamer::generateStream, this));
    }

    waitForFirstFrame();
}

UFFStreamer::UFFStreamer() {
    m_envelopeAndLogCompressor = EnvelopeAndLogCompressor::create();
    m_scanConverter = ScanConverter::create(1024, 1024)
            ->connect(m_envelopeAndLogCompressor);
	createOutputPort(0, "Image");
	m_loop = false;
	m_framerate = 30;

	createStringAttribute("filename", "Filename", "File to stream UFF data from", "");
	createStringAttribute("name", "Group name", "Name of which beamformed_data group to stream from", "");
	createBooleanAttribute("loop", "Loop", "Loop recordin", false);
}

UFFStreamer::UFFStreamer(std::string filename, bool loop, uint framerate, float gain, float dynamicRange, int width, int height, bool scanConvert) {
    m_envelopeAndLogCompressor = EnvelopeAndLogCompressor::create();
    m_scanConverter = ScanConverter::create(width, height)
            ->connect(m_envelopeAndLogCompressor);
    createOutputPort(0, "Image");
    if(filename.empty())
        throw Exception("You must give a filename to the UFF streamer");
    setFilename(filename);
    setLooping(loop);
    setFramerate(framerate);
    setGain(gain);
    setDynamicRange(dynamicRange);
    m_doScanConversion = scanConvert;
}

void UFFStreamer::loadAttributes() {
	setFilename(getStringAttribute("filename"));
	setLooping(getBooleanAttribute("loop"));
	setName(getStringAttribute("name"));
}

void UFFStreamer::setFilename(std::string filename) {
	m_filename = filename;
	setModified(true);
}

void UFFStreamer::setName(std::string name) {
	m_name = name;
	setModified(true);
}

void UFFStreamer::generateStream() {
    auto previousTime = std::chrono::high_resolution_clock::now();
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        m_currentFrameIndex = 0;
    }

    while (true){
        bool pause = getPause();
        if(pause)
            waitForUnpause();
        pause = getPause();

        {
            std::lock_guard<std::mutex> lock(m_stopMutex);
            if(m_stop) break;
        }

        int frameNr = getCurrentFrameIndex();

        // DO SCAN CONVERSION ETC.
        float startRadius = m_uffData->depth_axis.front();
        float stopRadius = m_uffData->depth_axis.back();
        float startTheta = m_uffData->azimuth_axis.front();
        float stopTheta = m_uffData->azimuth_axis.back();

        Image::pointer image ;
        if(m_uffData->hasGrayscaleData()) {
            image = m_uffData->dataScanconverted[frameNr];
        } else {
            image = m_uffData->iqData[frameNr];
        }
        image->updateModifiedTimestamp();

        float startX, startY, stopX, stopY, notUsed;
        if(m_uffData->polarCoordinates) {
            pol2cart(startRadius, startTheta, startY, notUsed);
            pol2cart(stopRadius, startTheta, notUsed, startX);
            pol2cart(stopRadius, 0, stopY, notUsed);
            pol2cart(stopRadius, stopTheta, notUsed, stopX);
            image->setFrameData("isPolar", "true");
        } else {
            startX = startTheta;
            stopX = stopTheta;
            startY = startRadius;
            stopY = stopRadius;
            image->setFrameData("isPolar", "false");
        }

        float newXSpacing = (stopX - startX) / (m_scanConverter->getWidth() - 1); //Subtract 1 because num spaces is 1 less than num elements
        float newYSpacing = (stopY - startY) / (m_scanConverter->getHeight() - 1);

        image->setFrameData("startRadius", std::to_string(startRadius));
        image->setFrameData("stopRadius", std::to_string(stopRadius));
        image->setFrameData("startTheta", std::to_string(startTheta));
        image->setFrameData("stopTheta", std::to_string(stopTheta));
        image->setFrameData("depthSpacing", std::to_string(m_uffData->depth_axis[1]-m_uffData->depth_axis[0]));
        image->setFrameData("azimuthSpacing", std::to_string(m_uffData->azimuth_axis[1]-m_uffData->azimuth_axis[0]));

        Image::pointer resultImage;
        if(m_doScanConversion) {
            // Do scan conversion
            if(m_uffData->hasGrayscaleData()) {
                m_scanConverter->connect(image);
            } else {
                // We must perform envelope and log compression
                m_envelopeAndLogCompressor->connect(image);
            }
            resultImage = m_scanConverter->runAndGetOutputData<Image>();
        } else {
            // Skip scan conversion
            if(m_uffData->hasGrayscaleData()) {
                //resultImage = image;
                // This is a hack to make UFFStreamer work with InterleavePlayback
                resultImage = image->copy(getMainDevice());
                resultImage->setFrameData(image->getFrameData());
            } else {
                // We must perform envelope and log compression
                m_envelopeAndLogCompressor->connect(image);
                resultImage = m_envelopeAndLogCompressor->runAndGetOutputData<Image>();
            }
        }

        if(!pause) {
            std::chrono::duration<float, std::milli> passedTime = std::chrono::high_resolution_clock::now() - previousTime;
            if(m_framerate > 0) {
                std::chrono::duration<int, std::milli> sleepFor(1000 / m_framerate - (int)passedTime.count());
                if(sleepFor.count() > 0)
                    std::this_thread::sleep_for(sleepFor);
            }
            previousTime = std::chrono::high_resolution_clock::now();
            getCurrentFrameIndexAndUpdate(); // Update
        }
        try {
            addOutputData(0, resultImage);
            frameAdded();
        } catch(ThreadStopped & e) {
            break;
        }
    }
}

UFFStreamer::~UFFStreamer() {
	stop();
}

int UFFStreamer::getNrOfFrames() {
    load();
	return m_uffData->numFrames;
}

void UFFStreamer::setGain(float gain) {
    m_scanConverter->setGain(gain);
    setModified(true);
}

void UFFStreamer::setDynamicRange(float dynamicRange) {
    m_scanConverter->setDynamicRange(dynamicRange);
    setModified(true);
}

}