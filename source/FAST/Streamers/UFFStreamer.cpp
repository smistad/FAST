#include "UFFStreamer.hpp"
#include <FAST/Data/Image.hpp>
#define H5_BUILT_AS_DYNAMIC_LIB
#include <H5Cpp.h>

namespace fast {


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
        std::string errorMessage = "No beamformed_data class group found";
        std::cout << errorMessage << std::endl;
        throw errorMessage;
    }
    selectedGroupName = beamformedDataGroups[0];
    return selectedGroupName;
}

UFFData UFFReader::getUFFData() {
    UFFData retVal;
    retVal.groupName = findHDF5BeamformedDataGroupName();

    H5::Group scanGroup = mFile.openGroup(retVal.groupName + "/scan");
    //NB: Function order matters
    getAxisNames(scanGroup, retVal);
    getImageSize(scanGroup, retVal);

    H5::Group dataGroup = getDataGroupAndIsScanconverted(retVal);
    getSpacing(scanGroup, retVal);

    readData(dataGroup, retVal);

    return retVal;
}

void UFFReader::getAxisNames(H5::Group scanGroup, UFFData &dataStruct) {
    auto classAttribute = scanGroup.openAttribute("class");
    auto className = readStringAttribute(classAttribute);

    if (className == "uff.linear_scan") {
        dataStruct.x_axis_name = "x_axis";
        dataStruct.y_axis_name = "z_axis";
        dataStruct.polarCoordinates = false;
    } else {
        dataStruct.x_axis_name = "azimuth_axis";
        dataStruct.y_axis_name = "depth_axis";
        dataStruct.polarCoordinates = true;
    }
    std::cout << "UFF axis names " << dataStruct.x_axis_name << " " << dataStruct.y_axis_name << std::endl;
}

void UFFReader::getImageSize(H5::Group scanGroup, UFFData &dataStruct) {
    {
        auto dataset = scanGroup.openDataSet(dataStruct.x_axis_name);
        auto dataspace = dataset.getSpace();
        hsize_t dims_out[2];
        int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
        dataStruct.width = dims_out[1];
    }
    {
        auto dataset = scanGroup.openDataSet(dataStruct.y_axis_name);
        auto dataspace = dataset.getSpace();
        hsize_t dims_out[2];
        int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
        dataStruct.height = dims_out[1];
    }
    std::cout << "UFF Image size was found to be " << dataStruct.width << " " << dataStruct.height << std::endl;
}

void UFFReader::getSpacing(H5::Group scanGroup, UFFData &dataStruct) {
    {
        //TODO: Verify that all spacings are equal if dataStruct.spacing is to be used

        //Should we use the "spacing vectors" for scanconverted data as well?
        //Now the data is read into azimuth_axis and depth_axis in both cases, but the
        //variable names don't sense for the scanvonverted case.

        auto dataset = scanGroup.openDataSet(dataStruct.x_axis_name);
        auto x_axis = std::make_unique<float[]>(dataStruct.width);
        dataset.read(x_axis.get(), H5::PredType::NATIVE_FLOAT);
        if(dataStruct.isScanconverted)
            dataStruct.spacing.x = std::fabs(x_axis[0] - x_axis[1])*1000;
        else
            dataStruct.spacing.x = std::fabs(x_axis[0] - x_axis[1]);//Don't multiply radians with 1000

        dataStruct.azimuth_axis.resize(dataStruct.width);
        for(int i = 0; i < dataStruct.width; ++i) {
            dataStruct.azimuth_axis[i] = x_axis[i];
        }
    }
    {
        auto dataset = scanGroup.openDataSet(dataStruct.y_axis_name);
        auto z_axis = std::make_unique<float[]>(dataStruct.height);
        dataset.read(z_axis.get(), H5::PredType::NATIVE_FLOAT);

        dataStruct.spacing.y = std::fabs(z_axis[0] - z_axis[1])*1000;

        dataStruct.depth_axis.resize(dataStruct.height);
        for(int i = 0; i < dataStruct.height; ++i) {
            dataStruct.depth_axis[i] = z_axis[i];
        }
    }
    std::cout << "Spacing in UFF file was " << dataStruct.spacing.x << " " << dataStruct.spacing.y << std::endl;
}

H5::Group UFFReader::getDataGroupAndIsScanconverted(UFFData &dataStruct) {
    H5::Group dataGroup;
    dataStruct.dataGroupName = dataStruct.groupName + "/data";
    dataStruct.isScanconverted = false;

    // Save old error handler
    H5E_auto2_t old_func;
    void *old_client_data;
    hid_t error_stack = H5E_DEFAULT;
    H5Eget_auto2( error_stack, &old_func, &old_client_data);

    // Turn off error handling to avoid error output from try - catch
    H5Eset_auto2(error_stack, NULL, NULL);

    try {
        dataGroup = mFile.openGroup(dataStruct.dataGroupName);
    }
    catch (...) {
        dataStruct.dataGroupName = dataStruct.groupName;
        dataGroup = mFile.openGroup(dataStruct.dataGroupName);
        dataStruct.isScanconverted = true;
    }

    // Restore previous error handler
    H5Eset_auto2(error_stack, old_func, old_client_data);

    std::cout << "UFF Image data " << ((dataStruct.isScanconverted) ? "is" : "is not") << " scanconverted." << std::endl;
    return dataGroup;
}

void UFFReader::readData(H5::Group dataGroup, UFFData &dataStruct) {

    if(dataStruct.isScanconverted)
        readScanconvertedData(dataGroup, dataStruct);
    else
        readNotScanconvertedData(dataGroup, dataStruct);
}


void UFFReader::readNotScanconvertedData(H5::Group dataGroup, UFFData &dataStruct) {
    auto dataset = dataGroup.openDataSet("imag");
    auto dataspace = dataset.getSpace();
    hsize_t dims_out[4];
    int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
    if (ndims != 4) {
        std::string errorMessage = "Exepected 4 dimensions in UFF file, got " + std::to_string(ndims);
        std::cout << errorMessage << std::endl;
        throw errorMessage;
    }
    int frameCount = dims_out[0];
    std::cout << "Nr of frames in UFF file: " << frameCount << std::endl;
    dataStruct.numFrames = frameCount;//TODO

    auto imagDataset = dataGroup.openDataSet("imag");
    auto imagDataspace = imagDataset.getSpace();
    auto realDataset = dataGroup.openDataSet("real");
    auto realDataspace = realDataset.getSpace();

    hsize_t count[4] = { 1, 1, 1, 1 }; // how many blocks to extract
    hsize_t blockSize[4] = { 1, 1, 1, hsize_t(dataStruct.width * dataStruct.height) }; // block
    hsize_t offset[4] = { 0, 0, 0, 0 };   // hyperslab offset in the file

    H5::DataSpace memspace(4, blockSize);

    //dataStruct.dataNotScanconverted.resize(frameCount);//Make room for pointers in vector
    dataStruct.iqData.resize(frameCount);//Make room for pointers in vector

    for (int frameNr = 0; frameNr < frameCount; ++frameNr) {
        //std::cout << "Extracting frame " << frameNr << " in UFF file" << std::endl;
        // Extract 1 frame
        auto imaginary = std::make_unique<float[]>(dataStruct.width * dataStruct.height);
        auto real = std::make_unique<float[]>(dataStruct.width * dataStruct.height);
        offset[0] = frameNr;
        imagDataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
        imagDataset.read(imaginary.get(), H5::PredType::NATIVE_FLOAT, memspace, imagDataspace);
        realDataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
        realDataset.read(real.get(), H5::PredType::NATIVE_FLOAT, memspace, realDataspace);

        //std::cout << "Start reading values" << std::endl;
        // TODO start env
        int dataSize = dataStruct.width * dataStruct.height;
        //std::vector<float> image_data;
        //image_data.resize(dataSize);
        std::vector<std::complex<double> > complex_image;
        complex_image.resize(dataSize);

        for (int y = 0; y < dataStruct.height; ++y) {
            for (int x = 0; x < dataStruct.width; ++x) {
                //TODO: Should axes be swapped?
                int pos = y + x * dataStruct.height;
                //image_data[x + y * dataStruct.width] = std::sqrt(imaginary[pos] * imaginary[pos] + real[pos] * real[pos]);
                complex_image[x + y * dataStruct.width] = std::complex<double>(real[pos],imaginary[pos]);
            }
        }
        // TODO end env
        //std::cout << "Got values. Move data to UFFData struct" << std::endl;
        //dataStruct.dataNotScanconverted[frameNr] = image_data;
        dataStruct.iqData[frameNr] = complex_image;

    }//for
}

void UFFReader::readScanconvertedData(H5::Group dataGroup, UFFData &dataStruct) {
    auto dataset = dataGroup.openDataSet("data");
    auto dataspace = dataset.getSpace();
    hsize_t dims_out[4];
    int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
    if (ndims != 4) {
        std::string errorMessage = "Exepected 4 dimensions in UFF file, got " + std::to_string(ndims);
        std::cout << errorMessage << std::endl;
        throw errorMessage;
    }
    int frameCount = dims_out[0];
    std::cout << "Nr of frames in UFF file: " << frameCount << std::endl;
    dataStruct.numFrames = frameCount;

    hsize_t count[4] = { 1, 1, 1, 1 }; // how many blocks to extract
    hsize_t blockSize[4] = { 1, 1, 1, hsize_t(dataStruct.width * dataStruct.height) }; // block
    hsize_t offset[4] = { 0, 0, 0, 0 };   // hyperslab offset in the file

    H5::DataSpace memspace(4, blockSize);

    dataStruct.dataScanconverted.resize(frameCount);//Make room for pointers in vector

    for (int frameNr = 0; frameNr < frameCount; ++frameNr) {
        //std::cout << "Extracting frame " << frameNr << " in UFF file" << std::endl;
        // Extract 1 frame
        auto data = std::make_unique<unsigned char[]>(dataStruct.width * dataStruct.height);
        offset[0] = frameNr;
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
        dataset.read(data.get(), H5::PredType::NATIVE_UCHAR, memspace, dataspace);

        int dataSize = dataStruct.width * dataStruct.height;
        std::vector<unsigned char> image_data;
        image_data.resize(dataSize);
        for (int y = 0; y < dataStruct.height; ++y) {
            for (int x = 0; x < dataStruct.width; ++x) {
                //TODO: Should axes be swapped?
                int pos = y + x * dataStruct.height;
                image_data[x + y * dataStruct.width] = data[pos];
            }
        }
        dataStruct.dataScanconverted[frameNr] = image_data;
    }//for
}

// UFFStreamer

UFFStreamer::UFFStreamer() {
    createOutputPort<Image>(0);
    m_loop = false;

    createStringAttribute("filename", "Filename", "File to stream UFF data from", "");
    createStringAttribute("name", "Group name", "Name of which beamformed_data group to stream from", "");
    createBooleanAttribute("loop", "Loop", "Loop recordin", false);
}

void UFFStreamer::loadAttributes() {
    setFilename(getStringAttribute("filename"));
    setLooping(getBooleanAttribute("loop"));
    setName(getStringAttribute("name"));
}

void UFFStreamer::setLooping(bool loop) {
    m_loop = loop;
}

void UFFStreamer::setFilename(std::string filename) {
    m_filename = filename;
    setModified(true);
}

void UFFStreamer::setName(std::string name) {
    m_name = name;
    setModified(true);
}

void UFFStreamer::execute() {
    if(!m_streamIsStarted) {
        if (m_filename.empty())
            throw Exception("You must set filename in UFFImageImporter with setFilename()");
        if (!fileExists(m_filename))
            throw FileNotFoundException(m_filename);

        m_streamIsStarted = true;
        m_thread = std::make_unique<std::thread>(std::bind(&UFFStreamer::generateStream, this));
    }

    waitForFirstFrame();
}

void UFFStreamer::generateStream() {
    auto uffReader = UFFReader();
    uffReader.open(m_filename);
    UFFData uffData = uffReader.getUFFData();

    if(uffData.isScanconverted) {
        while(true) {
            {
                std::lock_guard<std::mutex> lock(m_stopMutex);
                if(m_stop) break;
            }
            for(int frameNr = 0; frameNr < uffData.numFrames; ++frameNr) {
                auto image = Image::New();
                image->create(uffData.width, uffData.height, DataType::TYPE_UINT8, 1, uffData.dataScanconverted[frameNr].data());
                image->setSpacing(uffData.spacing.x, uffData.spacing.y, uffData.spacing.z);

                try {
                    addOutputData(0, image);
                    frameAdded();
                } catch(ThreadStopped & e) {
                    break;
                }
            }

            if(!m_loop) {
                break;
            }
        }
    } else {
        ScanConvertion scanconverter;
        scanconverter.loadData(uffData);

        scanconverter.scanConvert(512, 512, false);
        uffData = scanconverter.getUffData();

        for(int frameNr = 0; frameNr < uffData.numFrames; ++frameNr) {
            auto image = Image::New();
            image->create(uffData.width, uffData.height, DataType::TYPE_UINT8, 1, uffData.dataScanconverted[frameNr].data());
            image->setSpacing(uffData.spacing.x, uffData.spacing.y, uffData.spacing.z);
            addOutputData(0, image);
            frameAdded();
        }
    }
}

//
//void UFFStreamer::generateStream() {
//
//
//    try {
//        // Open file
//        H5::H5File file(m_filename.c_str(), H5F_ACC_RDONLY);
//
//        std::string selectedGroupName = m_name;
//        if(m_name.empty()) {
//            // Find HDF5 group name auomatically
//            std::vector<std::string> groupNames;
//            std::vector<std::string> beamformedDataGroups;
//            herr_t idx = H5Literate(file.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, file_info,
//                &groupNames);
//            for(auto groupName : groupNames) {
//                auto group = file.openGroup(groupName);
//                auto classAttribute = group.openAttribute("class");
//                auto className = readStringAttribute(classAttribute);
//                if(className == "uff.beamformed_data")
//                    beamformedDataGroups.push_back(groupName);
//            }
//            if(beamformedDataGroups.empty())
//                throw Exception("No beamformed_data class group found");
//            selectedGroupName = beamformedDataGroups[0];
//        }
//        reportInfo() << "Using HDF5 group: " << selectedGroupName << reportEnd();
//
//        int width;
//        int height;
//
//        auto scanGroup = file.openGroup(selectedGroupName + "/scan");
//        auto classAttribute = scanGroup.openAttribute("class");
//        auto className = readStringAttribute(classAttribute);
//        std::string x_axis_name, y_axis_name;
//        if(className == "uff.linear_scan") {
//            x_axis_name = "x_axis";
//            y_axis_name = "z_axis";
//        } else {
//            x_axis_name = "azimuth_axis";
//            y_axis_name = "depth_axis";
//        }
//        // First get image size
//        {
//            auto dataset = scanGroup.openDataSet(x_axis_name);
//            auto dataspace = dataset.getSpace();
//            hsize_t dims_out[2];
//            int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
//            width = dims_out[1];
//        }
//        {
//            auto dataset = scanGroup.openDataSet(y_axis_name);
//            auto dataspace = dataset.getSpace();
//            hsize_t dims_out[2];
//            int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
//            height = dims_out[1];
//        }
//        reportInfo() << "UFF Image size was found to be " << width << " " << height << reportEnd();
//
//        // Get spacing
//        Vector3f spacing = Vector3f::Ones();
//        {
//            // TODO only read 2 elements
//            auto dataset = scanGroup.openDataSet(x_axis_name);
//            auto x_axis = std::make_unique<float[]>(width);
//            dataset.read(x_axis.get(), H5::PredType::NATIVE_FLOAT);
//            spacing.x() = std::fabs(x_axis[0] - x_axis[1]) * 1000;
//        }
//        {
//            // TODO only read 2 elements
//            auto dataset = scanGroup.openDataSet(y_axis_name);
//            auto z_axis = std::make_unique<float[]>(height);
//            dataset.read(z_axis.get(), H5::PredType::NATIVE_FLOAT);
//            spacing.y() = std::fabs(z_axis[0] - z_axis[1]) * 1000;
//        }
//        reportInfo() << "Spacing in UFF file was " << spacing.transpose() << reportEnd();
//
//        H5::Group group;
//        bool scanconverted = false;
//        try {
//            group = file.openGroup(selectedGroupName + "/data");
//        } catch(...) {
//            group = file.openGroup(selectedGroupName);
//            scanconverted = true;
//        }
//
//        auto previousTime = std::chrono::high_resolution_clock::now();
//
//        // TODO refactor duplication of code
//        if(!scanconverted) {
//            auto dataset = group.openDataSet("imag");
//            auto dataspace = dataset.getSpace();
//            hsize_t dims_out[4];
//            int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
//            if(ndims != 4)
//                throw Exception("Exepected 4 dimensions in UFF file, got " + std::to_string(ndims));
//            int frameCount = dims_out[0];
//            reportInfo() << "Nr of frames in UFF file: " << frameCount << reportEnd();
//
//            auto imagDataset = group.openDataSet("imag");
//            auto imagDataspace = imagDataset.getSpace();
//            auto realDataset = group.openDataSet("real");
//            auto realDataspace = realDataset.getSpace();
//
//            hsize_t count[4] = {1, 1, 1, 1}; // how many blocks to extract
//            hsize_t blockSize[4] = {1, 1, 1, width * height}; // block
//            hsize_t offset[4] = {0, 0, 0, 0};   // hyperslab offset in the file
//
//            H5::DataSpace memspace(4, blockSize);
//
//            while(true) {
//                {
//                    std::lock_guard<std::mutex> lock(m_stopMutex);
//                    if(m_stop) break;
//                }
//                for(int frameNr = 0; frameNr < frameCount; ++frameNr) {
//                    {
//                        std::lock_guard<std::mutex> lock(m_stopMutex);
//                        if(m_stop) break;
//                    }
//                    reportInfo() << "Extracting frame " << frameNr << " in UFF file" << reportEnd();
//                    // Extract 1 frame
//                    auto imaginary = std::make_unique<float[]>(width * height);
//                    auto real = std::make_unique<float[]>(width * height);
//                    offset[0] = frameNr;
//                    imagDataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
//                    imagDataset.read(imaginary.get(), H5::PredType::NATIVE_FLOAT, memspace, imagDataspace);
//                    realDataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
//                    realDataset.read(real.get(), H5::PredType::NATIVE_FLOAT, memspace, realDataspace);
//
//                    // TODO start env
//                    auto image_data = std::make_unique<float[]>(width * height);
//                    for(int y = 0; y < height; ++y) {
//                        for(int x = 0; x < width; ++x) {
//
//                            int pos = y + x * height;
//                            image_data[x + y * width] = std::sqrt(imaginary[pos] * imaginary[pos] + real[pos] * real[pos]);
//                        }
//                    }
//                    // TODO end env
//                    auto image = Image::New();
//                    image->create(width, height, DataType::TYPE_FLOAT, 1, std::move(image_data));
//                    //image->setSpacing(spacing);
//                    //std::cout << image->calculateMaximumIntensity() << " " << image->calculateMinimumIntensity() << std::endl;
//
//                    std::chrono::duration<float, std::milli> passedTime = std::chrono::high_resolution_clock::now() - previousTime;
//                    std::chrono::duration<int, std::milli> sleepFor(1000 / m_framerate - (int)passedTime.count());
//                    if(sleepFor.count() > 0)
//                        std::this_thread::sleep_for(sleepFor);
//                    previousTime = std::chrono::high_resolution_clock::now();
//                    try {
//                        addOutputData(0, image);
//                        frameAdded();
//                    } catch(ThreadStopped & e) {
//                        break;
//                    }
//                }
//                if(!m_loop) {
//                    break;
//                }
//            }
//        } else {
//            auto dataset = group.openDataSet("data");
//            auto dataspace = dataset.getSpace();
//            hsize_t dims_out[4];
//            int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
//            if(ndims != 4)
//                throw Exception("Exepected 4 dimensions in UFF file, got " + std::to_string(ndims));
//            int frameCount = dims_out[0];
//            reportInfo() << "Nr of frames in UFF file: " << frameCount << reportEnd();
//
//            hsize_t count[4] = {1, 1, 1, 1}; // how many blocks to extract
//            hsize_t blockSize[4] = {1, 1, 1, width * height}; // block
//            hsize_t offset[4] = {0, 0, 0, 0};   // hyperslab offset in the file
//
//            H5::DataSpace memspace(4, blockSize);
//
//            while(true) {
//                {
//                    std::lock_guard<std::mutex> lock(m_stopMutex);
//                    if(m_stop) break;
//                }
//                for(int frameNr = 0; frameNr < frameCount; ++frameNr) {
//                    {
//                        std::lock_guard<std::mutex> lock(m_stopMutex);
//                        if(m_stop) break;
//                    }
//                    reportInfo() << "Extracting frame " << frameNr << " in UFF file" << reportEnd();
//                    // Extract 1 frame
//                    auto data = std::make_unique<uchar[]>(width * height);
//                    offset[0] = frameNr;
//                    dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
//                    dataset.read(data.get(), H5::PredType::NATIVE_UCHAR, memspace, dataspace);
//
//                    auto image_data = std::make_unique<uchar[]>(width * height);
//                    for(int y = 0; y < height; ++y) {
//                        for(int x = 0; x < width; ++x) {
//
//                            int pos = y + x * height;
//                            image_data[x + y * width] = data[pos];
//                        }
//                    }
//
//                    auto image = Image::New();
//                    image->create(width, height, DataType::TYPE_UINT8, 1, std::move(image_data));
//                    image->setSpacing(spacing);
//
//                    std::chrono::duration<float, std::milli> passedTime = std::chrono::high_resolution_clock::now() - previousTime;
//                    std::chrono::duration<int, std::milli> sleepFor(1000 / m_framerate - (int)passedTime.count());
//                    if(sleepFor.count() > 0)
//                        std::this_thread::sleep_for(sleepFor);
//                    previousTime = std::chrono::high_resolution_clock::now();
//                    try {
//                        addOutputData(0, image);
//                        frameAdded();
//                    } catch(ThreadStopped & e) {
//                        break;
//                    }
//                }
//                if(!m_loop) {
//                    break;
//                }
//            }
//        }
//        file.close();
//        reportInfo() << "UFF streamer stopped" << reportEnd();
//    } catch(H5::Exception & e) {
//        reportError() << "Error reading HDF5 file: " + e.getDetailMsg() << reportEnd();
//        throw Exception("Error reading HDF5 file: " + e.getDetailMsg());
//    } catch(H5::FileIException &e) {
//        reportError() << "Error reading HDF5 file: " + e.getDetailMsg() << reportEnd();
//        throw Exception("Error reading HDF5 file: " + e.getDetailMsg());
//    }
//}

UFFStreamer::~UFFStreamer() {
    {
        std::lock_guard<std::mutex> lock(m_stopMutex);
        m_stop = true;
    }
    m_thread->join();
}

void UFFStreamer::setFramerate(int framerate) {
    if(framerate <= 0)
        throw Exception("Framerate must be larger than 0");
    m_framerate = framerate;
}

double linearInterpolate(double a, double b, double t) {
    return a + t * (b -a );
}

//Convert from Polar to Cartesian Coordinates
//x = r × cos( θ )
//y = r × sin( θ )
//r = √ ( x2 + y2 )
//θ = tan-1 ( y / x )
void cart2pol(double x, double y, double &r, double &th) {
    r = sqrt(x*x + y*y);
    th = atan2(y,x);//Use atan2(), as atan(0/0) don't work
}

void pol2cart(double r, double th, double &x, double &y) {
    x = r * cos(th);
    y = r * sin(th);
}

// Normalize from dB image to 0 - 255 image and apply gain and dynamic range
unsigned int normalizeToGrayScale(double dBPixel, int dynamic_range, int gain) {
    double img_sc_reject = dBPixel + gain;
    img_sc_reject = (img_sc_reject < -dynamic_range) ? -dynamic_range : img_sc_reject; //Reject everything below dynamic range
    img_sc_reject = (img_sc_reject > 0) ? 0 : img_sc_reject; //Everything above 0 dB should be saturated
    unsigned int img_gray_scale = round(255*(img_sc_reject+dynamic_range)/dynamic_range);
    return img_gray_scale;
}

ScanConvertion::ScanConvertion() {

}


void ScanConvertion::loadData(UFFData &uffData) {
    mUFFData = uffData;
}

std::complex<double> ScanConvertion::findMax() {
    //TODO: Should the abs of the complex numbers be used for the comparison?

    //std::max_element needs a lambda function for the comparison
    auto absLess = [](const std::complex<double> &a,
                      const std::complex<double> &b) { return abs(a) < abs(b); };

    std::complex<double> max(0,0);

    for(int frame = 0; frame < mUFFData.numFrames; ++frame) {
        std::vector<std::complex<double> >::iterator maxIter;
        maxIter = std::max_element(mUFFData.iqData[frame].begin(),
                                   mUFFData.iqData[frame].end(),
                                   absLess);
        //std::cout << "maxIter: " << *maxIter << std::endl;
        if(absLess(max, *maxIter))
            max = *maxIter;
    }
    std::cout << "max complex number (iq): " << max << std::endl;
    return max;
}


void ScanConvertion::normalizeEnvelopeAndLogCompress() {
    mBeamData.resize(mUFFData.numFrames);//Store beam data in data structure as well?
    std::complex<double> max = findMax();

    for(int frame = 0; frame < mUFFData.numFrames; ++frame) {
        int numPixels = mUFFData.height * mUFFData.width;
        std::vector<double> beamImage;
        beamImage.resize(numPixels);

        for(int pixel = 0; pixel < numPixels; ++pixel) {
            //img_dB = 20*log10(abs(img./max(img(:)))); %Log compress, and normalize on max for all images

            std::complex<double> iq = mUFFData.iqData[frame][pixel];
            beamImage[pixel] = 20 * log10(abs(iq/max));
        }//for
        mBeamData[frame] = beamImage;
    }//for
}

bool ScanConvertion::scanConvert(int newWidth, int newHeight, bool linearInterpolation) {
    if(mUFFData.polarCoordinates)
        return scanConvertPolarCoordinates(newWidth, newHeight, linearInterpolation);
    else
        return scanConvertCartesianCoordinates(newWidth, newHeight, linearInterpolation);
}

bool ScanConvertion::scanConvertCartesianCoordinates(int newWidth, int newHeight, bool linearInterpolation) {
    if(mUFFData.isScanconverted)
        return false;
    if(mUFFData.polarCoordinates)
        return false;
    normalizeEnvelopeAndLogCompress();

    int frameSize = newWidth * newHeight;

    //X and Y values are stored in azimuth_axis and depth_axis for now
    //UFFData.x_axis_name and UFFData.y_axis_name should hold axis names
    double startY = mUFFData.depth_axis.front();
    double stopY = mUFFData.depth_axis.back();
    double startX = mUFFData.azimuth_axis.front();
    double stopX = mUFFData.azimuth_axis.back();

    double newXSpacing = (stopX - startX) / (newWidth - 1); //Subtract 1 because num spaces is 1 less than num elements
    double newYSpacing = (stopY - startY) / (newHeight - 1);

    mUFFData.dataScanconverted.resize(mUFFData.numFrames);//Make room for frames
    for(int frame = 0; frame < mUFFData.numFrames; ++frame) {

        std::vector<unsigned char> image_data;
        image_data.resize(frameSize);//Make room for frame

        for(int x = 0; x < newWidth; ++x) {
            for(int y = 0; y < newHeight; ++y) {
                double beamDataX = x * newXSpacing;
                double beamDataY = y * newYSpacing;
                double pixelValue_dB = getCartesianPixelValue(beamDataY, beamDataX, frame, linearInterpolation);
                unsigned int pixelValue = normalizeToGrayScale(pixelValue_dB);
                image_data[x + (y*newWidth)] = pixelValue;
            }
        }
        mUFFData.dataScanconverted[frame] = image_data;
    }//for


    mUFFData.isScanconverted = true;
    mUFFData.height = newHeight;
    mUFFData.width = newWidth;
    mUFFData.spacing.x = newXSpacing * 1000;
    mUFFData.spacing.y = newYSpacing * 1000;

    return true;
}

double ScanConvertion::getCartesianPixelValue(double xIq, double yIq, int frameNr, bool linear) {
    std::vector<double>::iterator yIter;
    std::vector<double>::iterator xIter;
    getIteratorToElementAfterValue(yIq, mUFFData.depth_axis, yIter);
    getIteratorToElementAfterValue(xIq, mUFFData.azimuth_axis, xIter);

    int yNum = yIter - mUFFData.depth_axis.begin();
    int xNum = xIter - mUFFData.azimuth_axis.begin();
    int pixelNum = yNum + (xNum * mUFFData.height);

    double pixelValue_dB = 0;
    if(linear) {
        double y = yIq;
        double x = xIq;
        double y1, y2, x1, x2;
        y2 = *yIter;
        if(yIter != mUFFData.depth_axis.begin())
            y1 = *(yIter - 1);
        else {
            std::cout << "Y iterator out of bounds" << std::endl;
            y1 = *(yIter + 1);
        }

        x2 = *xIter;
        if(xIter != mUFFData.azimuth_axis.begin())
            x1 = *(xIter - 1);
        else {
            std::cout << "X iterator out of bounds" << std::endl;
            x1 = *(xIter + 1);
        }

        //Find values of all 4 neighbours
        double row1Val1 = mBeamData[frameNr][pixelNum - 1];
        double row1Val2 = mBeamData[frameNr][pixelNum];
        double row2Val1 = mBeamData[frameNr][pixelNum - mUFFData.height - 1];
        double row2Val2 = mBeamData[frameNr][pixelNum - mUFFData.height];
        //std::cout << "Interpolate between: " << row1Val1 << " " << row1Val2 << " " << row2Val1 << " " << row2Val2 << std::endl;
        //Interpolation weight between r values and th values (bi-linear interpolation)
        double tY = (y - y1) / (y2 - y1);
        double tX = (x - x1) / (x2 - x1);
        double row1 = linearInterpolate(row1Val1, row1Val2, tX);
        double row2 = linearInterpolate(row2Val1, row2Val2, tX);
        pixelValue_dB = linearInterpolate(row1, row2, tY);
        //std::cout << "row1, row2, result " << row1 << " " << row2 << " " << pixelValue_dB << std::endl;
    } else {
        pixelValue_dB = mBeamData[frameNr][pixelNum];//Nearest Neighhbour (actually largest neighbour)
    }
    return pixelValue_dB;
}

bool ScanConvertion::scanConvertPolarCoordinates(int newWidth, int newHeight, bool linearInterpolation) {
    if(mUFFData.isScanconverted)
        return false;
    if(!mUFFData.polarCoordinates)
        return false;

    normalizeEnvelopeAndLogCompress();

    //TODO: Create lookuptable

    int numPixels = mUFFData.height * mUFFData.width;
    std::cout << "Convert from Polar to Cartesian Coordinates" << std::endl;

    //Use std::max_element/std::min_element instead of first/last?
    //Seems like it's not necessary
    //stopRadius = *std::max_element(mUFFData.depth_axis.begin(), mUFFData.depth_axis.end());
    double startRadius = mUFFData.depth_axis.front();
    double stopRadius = mUFFData.depth_axis.back();
    double startTheta = mUFFData.azimuth_axis.front();
    double stopTheta = mUFFData.azimuth_axis.back();

    std::cout << "radius: " << startRadius << " " << stopRadius << std::endl;
    std::cout << "theta: " << startTheta << " " << stopTheta << std::endl;

    // x and Y is swapped?
    double startX, startY, stopX, stopY, notUsed;
    pol2cart(startRadius, startTheta, startY, notUsed);
    pol2cart(stopRadius, startTheta, notUsed, startX);
    pol2cart(stopRadius, 0, stopY, notUsed);
    pol2cart(stopRadius, stopTheta, notUsed, stopX);
    std::cout << "X range: " << startX << " " << stopX << std::endl;
    std::cout << "Y range: " << startY << " " << stopY << std::endl;

    double newXSpacing = (stopX - startX) / (newWidth - 1); //Subtract 1 because num spaces is 1 less than num elements
    double newYSpacing = (stopY - startY) / (newHeight - 1);
    std::cout << "new spacing (m): " << newXSpacing << " " << newYSpacing << std::endl;

    int frameSize = newWidth * newHeight;

    mUFFData.dataScanconverted.resize(mUFFData.numFrames);//Make room for frames
    for(int frame = 0; frame < mUFFData.numFrames; ++frame) {

        std::vector<unsigned char> image_data;
        image_data.resize(frameSize);//Make room for frame

        //std::cout << int(uffData.dataNotScanconverted[i][pixelNum]) << " ";

        //std::cout << "starting frame coordinate conversion for frame: " << frame << std::endl;
        if(mUFFData.polarCoordinates) {
            for(int x = 0; x < newWidth; ++x) {
                for(int y = 0; y < newHeight; ++y) {

                    //Swap x and y? Not here maybe later?
                    double xPos = x*newXSpacing + startX;
                    double yPos = y*newYSpacing + startY;
//          double yPos = x*newXSpacing + startX;
//          double xPos = y*newYSpacing;

                    //Simple range check. Should not really fail
                    if((yPos < startY) || (yPos > stopY)) {
                        std::cout << "y range failed: " << yPos << std::endl;
                        continue;
                    }
                    if((xPos < startX) || (xPos > stopX)) {
                        std::cout << "x range failed: " << xPos << std::endl;
                        continue;
                    }

                    double r, th;
                    //cart2pol(xPos, yPos, r, th);
                    cart2pol(yPos, xPos, r, th);//Swap x and y

                    //Simple range check. May fail
                    if((r < startRadius) || (r > stopRadius)) {
                        //std::cout << "r range failed: " << r << std::endl;
                        continue;
                    }
                    if((th < startTheta) || (th > stopTheta)) {
                        //std::cout << "th range failed: " << th << std::endl;
                        continue;
                    }

                    double pixelValue_dB = getPixelValue(r, th, frame, linearInterpolation);
                    if(frame == 0) {
                        //std::cout << " x, y: " << x << " " << y << " theta: " << th << " r: " << r << " pixelNum: " << pixelNum << std::endl;
                    }
//          if(pixelNum < numPixels) {
                    //pixelValue_dB = mBeamData[frame][pixelNum];//Nearest Neighhbour
                    unsigned int pixelValue = normalizeToGrayScale(pixelValue_dB);
                    //std::cout << pixelValue_dB << " converted: " << pixelValue << std::endl;
                    image_data[x + (y*newWidth)] = pixelValue;
                    //image_data[y + (x*newHeight)] = pixelValue;//Test: swap axes
//          }
//          else
//            std::cout << "Error: Coordinate outside array (x,y): " << x << " " << y << std::endl;
                }//for
            }//for
        }//if
        mUFFData.dataScanconverted[frame] = image_data;
    }//for

    //Remove input (not scanconverted data)?
    mUFFData.isScanconverted = true;
    mUFFData.height = newHeight;
    mUFFData.width = newWidth;
    mUFFData.spacing.x = newXSpacing * 1000;
    mUFFData.spacing.y = newYSpacing * 1000;

    return true;
}

UFFData ScanConvertion::getUffData()
{
    return mUFFData;
}

double ScanConvertion::getPixelValue(double radius, double theta, int frameNr, bool linear) {
    std::vector<double>::iterator rIter;
    std::vector<double>::iterator thIter;
    getIteratorToElementAfterValue(radius, mUFFData.depth_axis, rIter);
    getIteratorToElementAfterValue(theta, mUFFData.azimuth_axis, thIter);

    //std::cout << "Interpolate r: " << r1 << " " << r2 << std::endl;
    //std::cout << "Interpolate th: " << th1 << " " << th2 << std::endl;


    int rNum = rIter - mUFFData.depth_axis.begin();
    int thNum = thIter - mUFFData.azimuth_axis.begin();
    int pixelNum = thNum + (rNum * mUFFData.width);

    double pixelValue_dB = 0;
    if(linear) {
        double r = radius;
        double th = theta;
        double r1, r2, th1, th2;
        r2 = *rIter;
        if(rIter != mUFFData.depth_axis.begin())
            r1 = *(rIter - 1);
        else {
            std::cout << "Radius iterator out of bounds" << std::endl;
            r1 = *(rIter + 1);
        }

        th2 = *thIter;
        if(thIter != mUFFData.azimuth_axis.begin())
            th1 = *(thIter - 1);
        else {
            std::cout << "Theta iterator out of bounds" << std::endl;
            th1 = *(thIter + 1);
        }

        //Find values of all 4 neighbours
        double row1Val1 = mBeamData[frameNr][pixelNum - 1];
        double row1Val2 = mBeamData[frameNr][pixelNum];
        double row2Val1 = mBeamData[frameNr][pixelNum - mUFFData.height - 1];
        double row2Val2 = mBeamData[frameNr][pixelNum - mUFFData.height];
        //std::cout << "Interpolate between: " << row1Val1 << " " << row1Val2 << " " << row2Val1 << " " << row2Val2 << std::endl;
        //Interpolation weight between r values and th values (bi-linear interpolation)
        double tR = (r - r1) / (r2 - r1);
        double tTh = (th - th1) / (th2 - th1);
        double row1 = linearInterpolate(row1Val1, row1Val2, tTh);
        double row2 = linearInterpolate(row2Val1, row2Val2, tTh);
        pixelValue_dB = linearInterpolate(row1, row2, tR);
        //std::cout << "row1, row2, result " << row1 << " " << row2 << " " << pixelValue_dB << std::endl;
    } else {
        pixelValue_dB = mBeamData[frameNr][pixelNum];//Nearest Neighhbour (actually largest neighbour)
    }
    return pixelValue_dB;
}

void ScanConvertion::getIteratorToElementAfterValue(double value, std::vector<double> &vector, std::vector<double>::iterator &iter) {
    iter = std::find_if(vector.begin(), vector.end(),
                        [value] (double element) {return (value < element);});
    if(iter == vector.end())
        iter += -1;
}

}