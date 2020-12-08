#pragma once

#include <FAST/Streamers/Streamer.hpp>
#include <H5Cpp.h>


namespace fast {

struct FAST_EXPORT UFFData{
    struct spacingStruct {
        double x = 0;
        double y = 0;
        double z = 0;
    };

    std::string groupName;
    std::string x_axis_name, y_axis_name;
    uint width;
    uint height;
    spacingStruct spacing;
    bool isScanconverted;
    std::string dataGroupName;
    int numFrames;
    bool polarCoordinates;
    std::vector<std::vector<unsigned char> > dataScanconverted;
    std::vector<std::vector<std::complex<double> > > iqData;

    std::vector<double> azimuth_axis;
    std::vector<double> depth_axis;
};

class UFFReader {
public:
    UFFReader();
    void open(std::string filename);
    void close();
    std::string findHDF5BeamformedDataGroupName();
    UFFData getUFFData();

private:
    H5::H5File mFile;

    void getAxisNames(H5::Group scanGroup, UFFData &dataStruct);
    void getImageSize(H5::Group scanGroup, UFFData &dataStruct);
    void getSpacing(H5::Group scanGroup, UFFData &dataStruct);
    H5::Group getDataGroupAndIsScanconverted(UFFData &dataStruct);

    void readData(H5::Group dataGroup, UFFData &dataStruct);
    void readNotScanconvertedData(H5::Group dataGroup, UFFData &dataStruct);
    void readScanconvertedData(H5::Group dataGroup, UFFData &dataStruct);

};

double linearInterpolate(double a, double b, double t);
void cart2pol(double x, double y, double &r, double &th);
void pol2cart(double r, double th, double &x, double &y);
unsigned int normalizeToGrayScale(double dBPixel, int dynamic_range = 60, int gain = 10);

class ScanConvertion {
public:
    ScanConvertion();
    void loadData(UFFData &uffData);
    bool scanConvert(int newWidth, int newHeight, bool linearInterpolation);
    UFFData getUffData();

protected:
    UFFData mUFFData;
    std::vector<std::vector<double> > mBeamData;

    bool scanConvertCartesianCoordinates(int newWidth = 512, int newHeight = 512, bool linearInterpolation = true);
    bool scanConvertPolarCoordinates(int newWidth = 512, int newHeight = 512, bool linearInterpolation = true);

    void normalizeEnvelopeAndLogCompress();
    std::complex<double> findMax();
    double getCartesianPixelValue(double xIq, double yIq, int frameNr, bool linear);
    double getPixelValue(double radius, double theta, int frameNr, bool linear = true);
    void getIteratorToElementAfterValue(double value, std::vector<double> &vector, std::vector<double>::iterator &iter);
};


class FAST_EXPORT UFFStreamer : public Streamer {
	FAST_OBJECT(UFFStreamer)
public:
	UFFStreamer();
	void setFilename(std::string filename);
	void execute() override;
	void setLooping(bool loop);
	// Set name of which HDF5 group to stream
	void setName(std::string name);
	void loadAttributes() override;
	void setFramerate(int framerate);
	~UFFStreamer();
protected:
	void generateStream() override;
	std::string m_filename;
	std::string m_name;
	bool m_loop;
	int m_framerate = 30;
};
}