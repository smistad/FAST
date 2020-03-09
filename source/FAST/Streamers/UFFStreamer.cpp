#include "UFFStreamer.hpp"
#include <FAST/Data/Image.hpp>
#define H5_BUILT_AS_DYNAMIC_LIB
#include <H5Cpp.h>

namespace fast {

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

    static std::string readStringAttribute(const H5::Attribute& att) {        
        std::string result;
        att.read(att.getDataType(), result);
        return result;
    }

    herr_t
        file_info(hid_t loc_id, const char* name, const H5L_info_t* linfo, void* opdata)
    {
        auto groupNames = (std::vector<std::string>*)opdata;
        groupNames->push_back(name);

        return 0;
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


        // Open file
        H5::H5File file(m_filename.c_str(), H5F_ACC_RDONLY);
        

        std::string selectedGroupName = m_name;
        if (m_name.empty()) {
            // Find HDF5 group name auomatically
            std::vector<std::string> groupNames;
            std::vector<std::string> beamformedDataGroups;
            herr_t idx = H5Literate(file.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, file_info,
                &groupNames);
            for (auto groupName : groupNames) {
                auto group = file.openGroup(groupName);
                auto classAttribute = group.openAttribute("class");
                auto className = readStringAttribute(classAttribute);
                if (className == "uff.beamformed_data")
                    beamformedDataGroups.push_back(groupName);
            }
            if (beamformedDataGroups.empty())
                throw Exception("No beamformed_data class group found");
            selectedGroupName = beamformedDataGroups[0];
        }
        reportInfo() << "Using HDF5 group: " << selectedGroupName << reportEnd();
       
        int width;
        int height;

        auto scanGroup = file.openGroup(selectedGroupName + "/scan");
        auto classAttribute = scanGroup.openAttribute("class");
        auto className = readStringAttribute(classAttribute);
        std::string x_axis_name, y_axis_name;
        if (className == "uff.linear_scan") {
            x_axis_name = "x_axis";
            y_axis_name = "z_axis";
        } else {
            x_axis_name = "azimuth_axis";
            y_axis_name = "depth_axis";
        }
        // First get image size
            {
                auto dataset = scanGroup.openDataSet(x_axis_name);
                auto dataspace = dataset.getSpace();
                hsize_t dims_out[2];
                int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
                width = dims_out[1];
            }
            {
                auto dataset = scanGroup.openDataSet(y_axis_name);
                auto dataspace = dataset.getSpace();
                hsize_t dims_out[2];
                int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
                height = dims_out[1];
            }     
            reportInfo() << "UFF Image size was found to be " << width << " " << height << reportEnd();

        // Get spacing
        Vector3f spacing = Vector3f::Ones();
        {
            // TODO only read 2 elements
            auto dataset = scanGroup.openDataSet(x_axis_name);
            auto x_axis = std::make_unique<float[]>(width);
            dataset.read(x_axis.get(), H5::PredType::NATIVE_FLOAT);
            spacing.x() = std::fabs(x_axis[0] - x_axis[1])*1000;
        }
        {
            // TODO only read 2 elements
            auto dataset = scanGroup.openDataSet(y_axis_name);
            auto z_axis = std::make_unique<float[]>(height);
            dataset.read(z_axis.get(), H5::PredType::NATIVE_FLOAT);
            spacing.y() = std::fabs(z_axis[0] - z_axis[1])*1000;
        }
        reportInfo() << "Spacing in UFF file was " << spacing.transpose() << reportEnd();
          
        H5::Group group;
        bool scanconverted = false;
        try {
            group = file.openGroup(selectedGroupName + "/data");
        }
        catch (...) {
            group = file.openGroup(selectedGroupName);
            scanconverted = true;
        }

        // TODO refactor duplication of code
        if (!scanconverted) {
            auto dataset = group.openDataSet("imag");
            auto dataspace = dataset.getSpace();
            hsize_t dims_out[4];
            int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
            if (ndims != 4)
                throw Exception("Exepected 4 dimensions in UFF file, got " + std::to_string(ndims));
            int frameCount = dims_out[0];
            reportInfo() << "Nr of frames in UFF file: " << frameCount << reportEnd();

            auto imagDataset = group.openDataSet("imag");
            auto imagDataspace = imagDataset.getSpace();
            auto realDataset = group.openDataSet("real");
            auto realDataspace = realDataset.getSpace();

            hsize_t count[4] = { 1, 1, 1, 1 }; // how many blocks to extract
            hsize_t blockSize[4] = { 1, 1, 1, width * height }; // block 
            hsize_t offset[4] = { 0, 0, 0, 0 };   // hyperslab offset in the file     

            H5::DataSpace memspace(4, blockSize);

            while (true) {
                for (int frameNr = 0; frameNr < frameCount; ++frameNr) {
                    reportInfo() << "Extracting frame " << frameNr << " in UFF file" << reportEnd();
                    // Extract 1 frame
                    auto imaginary = std::make_unique<float[]>(width * height);
                    auto real = std::make_unique<float[]>(width * height);
                    offset[0] = frameNr;
                    imagDataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
                    imagDataset.read(imaginary.get(), H5::PredType::NATIVE_FLOAT, memspace, imagDataspace);
                    realDataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
                    realDataset.read(real.get(), H5::PredType::NATIVE_FLOAT, memspace, realDataspace);

                    // TODO start env
                    auto image_data = std::make_unique<float[]>(width * height);
                    for (int y = 0; y < height; ++y) {
                        for (int x = 0; x < width; ++x) {

                            int pos = y + x * height;
                            image_data[x + y * width] = std::sqrt(imaginary[pos] * imaginary[pos] + real[pos] * real[pos]);
                        }
                    }
                    // TODO end env
                    auto image = Image::New();
                    image->create(width, height, DataType::TYPE_FLOAT, 1, std::move(image_data));
                    //image->setSpacing(spacing);
                    //std::cout << image->calculateMaximumIntensity() << " " << image->calculateMinimumIntensity() << std::endl;

                    try {
                        addOutputData(0, image);
                        frameAdded();
                    }
                    catch (ThreadStopped & e) {
                        break;
                    }
                }
                if (!m_loop) {
                    break;
                }
            }
        } else {
            auto dataset = group.openDataSet("data");
            auto dataspace = dataset.getSpace();
            hsize_t dims_out[4];
            int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
            if (ndims != 4)
                throw Exception("Exepected 4 dimensions in UFF file, got " + std::to_string(ndims));
            int frameCount = dims_out[0];
            reportInfo() << "Nr of frames in UFF file: " << frameCount << reportEnd();

            hsize_t count[4] = { 1, 1, 1, 1 }; // how many blocks to extract
            hsize_t blockSize[4] = { 1, 1, 1, width * height }; // block 
            hsize_t offset[4] = { 0, 0, 0, 0 };   // hyperslab offset in the file     

            H5::DataSpace memspace(4, blockSize);

            while (true) {
                for (int frameNr = 0; frameNr < frameCount; ++frameNr) {
                    reportInfo() << "Extracting frame " << frameNr << " in UFF file" << reportEnd();
                    // Extract 1 frame
                    auto data = std::make_unique<uchar[]>(width * height);
                    offset[0] = frameNr;
                    dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, NULL, blockSize);
                    dataset.read(data.get(), H5::PredType::NATIVE_UCHAR, memspace, dataspace);

                    auto image_data = std::make_unique<uchar[]>(width * height);
                    for (int y = 0; y < height; ++y) {
                        for (int x = 0; x < width; ++x) {

                            int pos = y + x * height;
                            image_data[x + y * width] = data[pos];
                        }
                    }

                    auto image = Image::New();
                    image->create(width, height, DataType::TYPE_UINT8, 1, std::move(image_data));
                    image->setSpacing(spacing);

                    try {
                        addOutputData(0, image);
                        frameAdded();
                    }
                    catch (ThreadStopped & e) {
                        break;
                    }
                }
                if (!m_loop) {
                    break;
                }
            }
        }
}

}