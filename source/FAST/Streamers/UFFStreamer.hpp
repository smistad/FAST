#pragma once

#include <FAST/Streamers/RandomAccessStreamer.hpp>


namespace fast {

class UFFReader;

double linearInterpolate(double a, double b, double t);
void cart2pol(double x, double y, double &r, double &th);
void pol2cart(double r, double th, double &x, double &y);
unsigned int normalizeToGrayScale(double dBPixel, int dynamic_range = 60, int gain = 10);


/**
* A data structure for the ultrasound file format (UFF).
* Stores relevant parameters.
*/

struct UFFData {
    Vector3f spacing;
    std::string groupName;
    std::string x_axis_name, y_axis_name;
    uint width;
    uint height;
    bool isScanConverted;
    std::string dataGroupName;
    int numFrames;
    bool polarCoordinates;
    std::vector<std::vector<unsigned char> > dataScanconverted;
    std::vector<std::vector<std::complex<double> > > iqData;

    std::vector<double> azimuth_axis;
    std::vector<double> depth_axis;
};


/**
* A streamer for reading data stored in the ultrasound file format (UFF)
* which is essentially and HDF5 file with ultrasound image/beam data.
*/
class FAST_EXPORT UFFStreamer : public RandomAccessStreamer {
    FAST_OBJECT(UFFStreamer)

    public:
        UFFStreamer();
        void setFilename(std::string filename);
        void execute() override;
        int getNrOfFrames() override;
        // Set name of which HDF5 group to stream
        void setName(std::string name);
        void loadAttributes() override;
        ~UFFStreamer();

    protected:
        void generateStream() override;
        std::string m_filename;
        std::string m_name;
        UFFData m_uffData;
};


/**
* A scan converter for data from the ultrasound file format (UFF).
*
*/
class UFFScanConvert {
    public:
        UFFScanConvert();
        void loadData(UFFData &uffData);
        bool scanConvert(int newWidth, int newHeight, bool linearInterpolation);
        UFFData getUffData();

    protected:
        UFFData m_uffData;
        std::vector<std::vector<double> > mBeamData;

        bool scanConvertCartesianCoordinates(int newWidth = 512, int newHeight = 512, bool linearInterpolation = true);
        bool scanConvertPolarCoordinates(int newWidth = 512, int newHeight = 512, bool linearInterpolation = true);
        void normalizeEnvelopeAndLogCompress();
        std::complex<double> findMax();
        double getCartesianPixelValue(double xIq, double yIq, int frameNr, bool linear);
        double getPixelValue(double radius, double theta, int frameNr, bool linear = true);
        void getIteratorToElementAfterValue(double value, std::vector<double> &vector, std::vector<double>::iterator &iter);
};


}