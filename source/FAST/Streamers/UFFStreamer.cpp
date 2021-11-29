#include "UFFStreamer.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Reporter.hpp>

#define H5_BUILT_AS_DYNAMIC_LIB
#include <H5Cpp.h>

namespace fast {


inline float linearInterpolate(float a, float b, float t);
inline void cart2pol(float x, float y, float &r, float &th);
inline void pol2cart(float r, float th, float &x, float &y);
inline uchar normalizeToGrayScale(float dBPixel, int dynamic_range = 60, int gain = 10);


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
    std::vector<std::unique_ptr<uchar[]>> dataScanconverted;
    std::vector<std::vector<std::complex<float> > > iqData;

    std::vector<float> azimuth_axis;
    std::vector<float> depth_axis;
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


/**
* A scan converter for data from the ultrasound file format (UFF).
*
*/
class UFFScanConvert {
    public:
        UFFScanConvert();
        void loadData(std::shared_ptr<UFFData> uffData);
        bool scanConvert(int newWidth, int newHeight, bool linearInterpolation);
        std::shared_ptr<UFFData> getUffData();

    protected:
        std::shared_ptr<UFFData> m_uffData;
        std::vector<std::unique_ptr<float[]>> mBeamData;

        bool scanConvertCartesianCoordinates(int newWidth = 512, int newHeight = 512, bool linearInterpolation = true);
        bool scanConvertPolarCoordinates(int newWidth = 512, int newHeight = 512, bool linearInterpolation = true);
        void normalizeEnvelopeAndLogCompress();
        std::complex<float> findMax();
        float getCartesianPixelValue(float xIq, float yIq, int frameNr, bool linear);
        float getPixelValue(float radius, float theta, int frameNr, bool linear = true);
        void getIteratorToElementAfterValue(float value, std::vector<float> &vector, std::vector<float>::iterator &iter);
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
        dataGroup = mFile.openGroup(dataStruct->dataGroupName);
    }
    catch (...) {
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

    if (ndims != 4) {
        throw Exception("Exepected 4 dimensions in UFF file, got " + std::to_string(ndims));
    }

    int frameCount = dims_out[0];
    std::cout << "Number of frames in UFF file: " << frameCount << std::endl;
    dataStruct->numFrames = frameCount;//TODO

    auto imagDataset = dataGroup.openDataSet("imag");
    auto imagDataspace = imagDataset.getSpace();
    auto realDataset = dataGroup.openDataSet("real");
    auto realDataspace = realDataset.getSpace();

    hsize_t count[4] = { 1, 1, 1, 1 }; // how many blocks to extract
    hsize_t blockSize[4] = { 1, 1, 1, hsize_t(dataStruct->width * dataStruct->height) }; // block
    hsize_t offset[4] = { 0, 0, 0, 0 };   // hyperslab offset in the file

    H5::DataSpace memspace(4, blockSize);

    //dataStruct.dataNotScanconverted.resize(frameCount);//Make room for pointers in vector
    dataStruct->iqData.resize(frameCount);//Make room for pointers in vector

    for (int frameNr = 0; frameNr < frameCount; ++frameNr) {
        //std::cout << "Extracting frame " << frameNr << " in UFF file" << std::endl;
        // Extract 1 frame
        auto imaginary = std::make_unique<float[]>(dataStruct->width * dataStruct->height);
        auto real = std::make_unique<float[]>(dataStruct->width * dataStruct->height);
        offset[0] = frameNr;
        imagDataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
        imagDataset.read(imaginary.get(), H5::PredType::NATIVE_FLOAT, memspace, imagDataspace);
        realDataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
        realDataset.read(real.get(), H5::PredType::NATIVE_FLOAT, memspace, realDataspace);

        //std::cout << "Start reading values" << std::endl;
        // TODO start env
        int dataSize = dataStruct->width * dataStruct->height;
        //std::vector<float> image_data;
        //image_data.resize(dataSize);
        std::vector<std::complex<float> > complex_image;
        complex_image.resize(dataSize);

        for (int y = 0; y < dataStruct->height; ++y) {
            for (int x = 0; x < dataStruct->width; ++x) {
                //TODO: Should axes be swapped?
                int pos = y + x * dataStruct->height;
                //image_data[x + y * dataStruct.width] = std::sqrt(imaginary[pos] * imaginary[pos] + real[pos] * real[pos]);
                complex_image[x + y * dataStruct->width] = std::complex<float>(real[pos],imaginary[pos]);
            }
        }
        // TODO end env
        //std::cout << "Got values. Move data to UFFData struct" << std::endl;
        //dataStruct.dataNotScanconverted[frameNr] = image_data;
        dataStruct->iqData[frameNr] = complex_image;

    }//for
}

void UFFReader::readScanconvertedData(H5::Group dataGroup, std::shared_ptr<UFFData> dataStruct) {
    auto dataset = dataGroup.openDataSet("data");
    auto dataspace = dataset.getSpace();
    hsize_t dims_out[4];
    int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);

    if (ndims != 4) {
        throw Exception("Exepected 4 dimensions in UFF file, got " + std::to_string(ndims));
    }

    int frameCount = dims_out[0];
    std::cout << "Number of frames in UFF file: " << frameCount << std::endl;

    dataStruct->numFrames = frameCount;

    hsize_t count[4] = { 1, 1, 1, 1 }; // how many blocks to extract
    hsize_t blockSize[4] = { 1, 1, 1, hsize_t(dataStruct->width * dataStruct->height) }; // block
    hsize_t offset[4] = { 0, 0, 0, 0 };   // hyperslab offset in the file

    H5::DataSpace memspace(4, blockSize);

    dataStruct->dataScanconverted.resize(frameCount);//Make room for pointers in vector

    for (int frameNr = 0; frameNr < frameCount; ++frameNr) {
        //std::cout << "Extracting frame " << frameNr << " in UFF file" << std::endl;
        // Extract 1 frame
        auto data = std::make_unique<unsigned char[]>(dataStruct->width * dataStruct->height);
        offset[0] = frameNr;
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
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
        dataStruct->dataScanconverted[frameNr] = std::move(image_data);
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
	createOutputPort(0, "Image");
	m_loop = false;
	m_framerate = 30;

	createStringAttribute("filename", "Filename", "File to stream UFF data from", "");
	createStringAttribute("name", "Group name", "Name of which beamformed_data group to stream from", "");
	createBooleanAttribute("loop", "Loop", "Loop recordin", false);
}

UFFStreamer::UFFStreamer(std::string filename, bool loop, uint framerate) {
    createOutputPort(0, "Image");
    setFilename(filename);
    setLooping(loop);
    setFramerate(framerate);
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

    if (m_uffData->isScanConverted) {

        while (true) {
            bool pause = getPause();
            if(pause)
                waitForUnpause();
            pause = getPause();

            {
                std::lock_guard<std::mutex> lock(m_stopMutex);
                if(m_stop) break;
            }

            int frameNr = getCurrentFrameIndexAndUpdate();

            auto image = Image::create(m_uffData->width, m_uffData->height, DataType::TYPE_UINT8, 1,
                          m_uffData->dataScanconverted[frameNr].get());
            image->setSpacing(m_uffData->spacing.x(), m_uffData->spacing.y(), m_uffData->spacing.z());

            if(!pause) {
                std::chrono::duration<float, std::milli> passedTime = std::chrono::high_resolution_clock::now() - previousTime;
                std::chrono::duration<int, std::milli> sleepFor(1000 / m_framerate - (int)passedTime.count());
                if(sleepFor.count() > 0)
                    std::this_thread::sleep_for(sleepFor);
                previousTime = std::chrono::high_resolution_clock::now();
            }
            try {
                addOutputData(0, image);
                frameAdded();
            } catch(ThreadStopped & e) {
                break;
            }
        }
    } else {
        UFFScanConvert scanconverter;
        scanconverter.loadData(m_uffData);

        mRuntimeManager->enable();
        mRuntimeManager->startRegularTimer("scan-convert");
        scanconverter.scanConvert(512, 512, true);
        m_uffData = scanconverter.getUffData();
        mRuntimeManager->stopRegularTimer("scan-convert");
        mRuntimeManager->print("scan-convert");

        while (true){
            bool pause = getPause();
            if(pause)
                waitForUnpause();
            pause = getPause();

            {
                std::lock_guard<std::mutex> lock(m_stopMutex);
                if(m_stop) break;
            }

            int frameNr = getCurrentFrameIndexAndUpdate();

            auto image = Image::create(m_uffData->width, m_uffData->height, DataType::TYPE_UINT8, 1,
                                       m_uffData->dataScanconverted[frameNr].get());
            image->setSpacing(m_uffData->spacing.x(), m_uffData->spacing.y(), m_uffData->spacing.z());
            if(!pause) {
                std::chrono::duration<float, std::milli> passedTime = std::chrono::high_resolution_clock::now() - previousTime;
                std::chrono::duration<int, std::milli> sleepFor(1000 / m_framerate - (int)passedTime.count());
                if(sleepFor.count() > 0)
                    std::this_thread::sleep_for(sleepFor);
                previousTime = std::chrono::high_resolution_clock::now();
            }
            try {
                addOutputData(0, image);
                frameAdded();
            } catch(ThreadStopped & e) {
                break;
            }
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

float linearInterpolate(float a, float b, float t) {
    return a + t * (b -a );
}

void cart2pol(float x, float y, float &r, float &th) {
    r = std::sqrt(x*x + y*y);
    th = std::atan2(y,x);//Use atan2(), as atan(0/0) don't work
}

void pol2cart(float r, float th, float &x, float &y) {
    x = r * std::cos(th);
    y = r * std::sin(th);
}

// Normalize from dB image to 0 - 255 image and apply gain and dynamic range
uchar normalizeToGrayScale(float dBPixel, int dynamic_range, int gain) {
    float img_sc_reject = dBPixel + gain;
    img_sc_reject = (img_sc_reject < -dynamic_range) ? -dynamic_range : img_sc_reject; //Reject everything below dynamic range
    img_sc_reject = (img_sc_reject > 0) ? 0 : img_sc_reject; //Everything above 0 dB should be saturated
    uchar img_gray_scale = round(255*(img_sc_reject+dynamic_range)/dynamic_range);
    return img_gray_scale;
}

UFFScanConvert::UFFScanConvert() {

}

void UFFScanConvert::loadData(std::shared_ptr<UFFData> uffData) {
    m_uffData = uffData;
}

std::complex<float> UFFScanConvert::findMax() {
    //TODO: Should the abs of the complex numbers be used for the comparison?

    //std::max_element needs a lambda function for the comparison
    auto absLess = [](const std::complex<float> &a,
                      const std::complex<float> &b) { return abs(a) < abs(b); };

    std::complex<float> max(0,0);

    for(int frame = 0; frame < m_uffData->numFrames; ++frame) {
        std::vector<std::complex<float> >::iterator maxIter;
        maxIter = std::max_element(m_uffData->iqData[frame].begin(), m_uffData->iqData[frame].end(), absLess);
        //std::cout << "maxIter: " << *maxIter << std::endl;
        if(absLess(max, *maxIter))
            max = *maxIter;
    }
    return max;
}


void UFFScanConvert::normalizeEnvelopeAndLogCompress() {
    mBeamData.resize(m_uffData->numFrames);//Store beam data in data structure as well?
    std::complex<float> max = findMax();

    for(int frame = 0; frame < m_uffData->numFrames; ++frame) {
        int numPixels = m_uffData->height * m_uffData->width;
        auto beamImage = make_uninitialized_unique<float[]>(numPixels);

        for(int pixel = 0; pixel < numPixels; ++pixel) {
            //img_dB = 20*log10(abs(img./max(img(:)))); %Log compress, and normalize on max for all images

            std::complex<float> iq = m_uffData->iqData[frame][pixel];
            beamImage[pixel] = 20.0f * std::log10(std::abs(iq/max));
        }//for
        mBeamData[frame] = std::move(beamImage);
    }//for
}

bool UFFScanConvert::scanConvert(int newWidth, int newHeight, bool linearInterpolation) {
    if(m_uffData->polarCoordinates)
        return scanConvertPolarCoordinates(newWidth, newHeight, linearInterpolation);
    else
        return scanConvertCartesianCoordinates(newWidth, newHeight, linearInterpolation);
}

bool UFFScanConvert::scanConvertCartesianCoordinates(int newWidth, int newHeight, bool linearInterpolation) {
    if(m_uffData->isScanConverted)
        return false;
    if(m_uffData->polarCoordinates)
        return false;
    normalizeEnvelopeAndLogCompress();

    int frameSize = newWidth * newHeight;

    //X and Y values are stored in azimuth_axis and depth_axis for now
    //UFFData.x_axis_name and UFFData.y_axis_name should hold axis names
    float startY = m_uffData->depth_axis.front();
    float stopY = m_uffData->depth_axis.back();
    float startX = m_uffData->azimuth_axis.front();
    float stopX = m_uffData->azimuth_axis.back();

    float newXSpacing = (stopX - startX) / (newWidth - 1); //Subtract 1 because num spaces is 1 less than num elements
    float newYSpacing = (stopY - startY) / (newHeight - 1);

    m_uffData->dataScanconverted.resize(m_uffData->numFrames);//Make room for frames
    for(int frame = 0; frame < m_uffData->numFrames; ++frame) {

        auto image_data = make_uninitialized_unique<uchar[]>(frameSize);

        for(int x = 0; x < newWidth; ++x) {
            for(int y = 0; y < newHeight; ++y) {
                float beamDataX = x * newXSpacing;
                float beamDataY = y * newYSpacing;
                float pixelValue_dB = getCartesianPixelValue(beamDataY, beamDataX, frame, linearInterpolation);
                uchar pixelValue = normalizeToGrayScale(pixelValue_dB);
                image_data[x + (y*newWidth)] = pixelValue;
            }
        }
        m_uffData->dataScanconverted[frame] = std::move(image_data);
    }

    m_uffData->isScanConverted = true;
    m_uffData->height = newHeight;
    m_uffData->width = newWidth;
    m_uffData->spacing.x() = newXSpacing * 1000;
    m_uffData->spacing.y() = newYSpacing * 1000;

    return true;
}

float UFFScanConvert::getCartesianPixelValue(float xIq, float yIq, int frameNr, bool linear) {
    std::vector<float>::iterator yIter;
    std::vector<float>::iterator xIter;
    getIteratorToElementAfterValue(yIq, m_uffData->depth_axis, yIter);
    getIteratorToElementAfterValue(xIq, m_uffData->azimuth_axis, xIter);

    int yNum = yIter - m_uffData->depth_axis.begin();
    int xNum = xIter - m_uffData->azimuth_axis.begin();
    int pixelNum = yNum + (xNum * m_uffData->height);

    float pixelValue_dB = 0;
    if(linear) {
        float y = yIq;
        float x = xIq;
        float y1, y2, x1, x2;
        y2 = *yIter;
        if(yIter != m_uffData->depth_axis.begin())
            y1 = *(yIter - 1);
        else {
            std::cout << "Y iterator out of bounds" << std::endl;
            y1 = *(yIter + 1);
        }

        x2 = *xIter;
        if(xIter != m_uffData->azimuth_axis.begin())
            x1 = *(xIter - 1);
        else {
            std::cout << "X iterator out of bounds" << std::endl;
            x1 = *(xIter + 1);
        }

        //Find values of all 4 neighbours
        float row1Val1 = mBeamData[frameNr][pixelNum - 1];
        float row1Val2 = mBeamData[frameNr][pixelNum];
        float row2Val1 = mBeamData[frameNr][pixelNum - (int)m_uffData->width - 1];
        float row2Val2 = mBeamData[frameNr][pixelNum - (int)m_uffData->width];

        float tY = (y - y1) / (y2 - y1);
        float tX = (x - x1) / (x2 - x1);
        float row1 = linearInterpolate(row1Val1, row1Val2, tX);
        float row2 = linearInterpolate(row2Val1, row2Val2, tX);
        pixelValue_dB = linearInterpolate(row1, row2, tY);
    } else {
        pixelValue_dB = mBeamData[frameNr][pixelNum]; //Nearest Neighbour (actually largest neighbour)
    }
    return pixelValue_dB;
}

bool UFFScanConvert::scanConvertPolarCoordinates(int newWidth, int newHeight, bool linearInterpolation) {
    if(m_uffData->isScanConverted)
        return false;
    if(!m_uffData->polarCoordinates)
        return false;

    normalizeEnvelopeAndLogCompress();

    //TODO: Create lookuptable

    int numPixels = m_uffData->height * m_uffData->width;

    //Use std::max_element/std::min_element instead of first/last?
    //Seems like it's not necessary
    float startRadius = m_uffData->depth_axis.front();
    float stopRadius = m_uffData->depth_axis.back();
    float startTheta = m_uffData->azimuth_axis.front();
    float stopTheta = m_uffData->azimuth_axis.back();

    // x and Y is swapped?
    float startX, startY, stopX, stopY, notUsed;
    pol2cart(startRadius, startTheta, startY, notUsed);
    pol2cart(stopRadius, startTheta, notUsed, startX);
    pol2cart(stopRadius, 0, stopY, notUsed);
    pol2cart(stopRadius, stopTheta, notUsed, stopX);

    float newXSpacing = (stopX - startX) / (newWidth - 1); //Subtract 1 because num spaces is 1 less than num elements
    float newYSpacing = (stopY - startY) / (newHeight - 1);

    int frameSize = newWidth * newHeight;

    m_uffData->dataScanconverted.resize(m_uffData->numFrames);//Make room for frames
    for(int frame = 0; frame < m_uffData->numFrames; ++frame) {

        auto image_data = make_uninitialized_unique<uchar[]>(frameSize);
        std::vector<float>::iterator rIter;
        std::vector<float>::iterator thIter;

        if(m_uffData->polarCoordinates) {
            for(int x = 0; x < newWidth; ++x) {
                for(int y = 0; y < newHeight; ++y) {

                    //Swap x and y? Not here maybe later?
                    float xPos = x*newXSpacing + startX;
                    float yPos = y*newYSpacing + startY;

                    float r, th;
                    cart2pol(yPos, xPos, r, th);//Swap x and y

                    //Simple range check. May fail
                    if((r < startRadius) || (r > stopRadius)) {
                        continue;
                    }

                    if((th < startTheta) || (th > stopTheta)) {
                        continue;
                    }

                    float pixelValue_dB = getPixelValue(r, th, frame, linearInterpolation); // This func eats a lot of runtime
                    uchar pixelValue = normalizeToGrayScale(pixelValue_dB);
                    image_data[x + (y*newWidth)] = pixelValue;
                }
            }
        }

        m_uffData->dataScanconverted[frame] = std::move(image_data);
    }

    //Remove input (not scanconverted data)?
    m_uffData->isScanConverted = true;
    m_uffData->height = newHeight;
    m_uffData->width = newWidth;
    m_uffData->spacing.x() = newXSpacing * 1000;
    m_uffData->spacing.y() = newYSpacing * 1000;

    return true;
}

std::shared_ptr<UFFData> UFFScanConvert::getUffData()
{
    return m_uffData;
}

float UFFScanConvert::getPixelValue(float radius, float theta, int frameNr, bool linear) {
    std::vector<float>::iterator rIter;
    std::vector<float>::iterator thIter;
    // TODO: These are very slow, why are they needed?:
    getIteratorToElementAfterValue(radius, m_uffData->depth_axis, rIter);
    getIteratorToElementAfterValue(theta, m_uffData->azimuth_axis, thIter);

    int rNum = rIter - m_uffData->depth_axis.begin();
    int thNum = thIter - m_uffData->azimuth_axis.begin();
    int pixelNum = thNum + (rNum * m_uffData->width);

    float pixelValue_dB = 0;
    if(linear) {
        float r = radius;
        float th = theta;
        float r1, r2, th1, th2;
        r2 = *rIter;
        if(rIter != m_uffData->depth_axis.begin())
            r1 = *(rIter - 1);
        else {
            std::cout << "Radius iterator out of bounds" << std::endl;
            r1 = *(rIter + 1);
        }

        th2 = *thIter;
        if(thIter != m_uffData->azimuth_axis.begin())
            th1 = *(thIter - 1);
        else {
            std::cout << "Theta iterator out of bounds" << std::endl;
            th1 = *(thIter + 1);
        }

        //Find values of all 4 neighbours
        float row1Val1 = mBeamData[frameNr][pixelNum - 1];
        float row1Val2 = mBeamData[frameNr][pixelNum];
        float row2Val1 = mBeamData[frameNr][pixelNum - (int)m_uffData->width - 1];
        float row2Val2 = mBeamData[frameNr][pixelNum - (int)m_uffData->width];

        //Interpolation weight between r values and th values (bi-linear interpolation)
        float tR = 1.0f - (r - r1) / (r2 - r1);
        float tTh = (th - th1) / (th2 - th1);
        float row1 = linearInterpolate(row1Val1, row1Val2, tTh);
        float row2 = linearInterpolate(row2Val1, row2Val2, tTh);
        pixelValue_dB = linearInterpolate(row1, row2, tR);
    } else {
        pixelValue_dB = mBeamData[frameNr][pixelNum]; //Nearest Neighbour (actually largest neighbour)
    }
    return pixelValue_dB;
}

void UFFScanConvert::getIteratorToElementAfterValue(float value, std::vector<float> &vector, std::vector<float>::iterator &iter) {
    iter = std::find_if(vector.begin(), vector.end(),
                        [value] (float element) {return (value < element);});
    if(iter == vector.end())
        iter += -1;
}

}