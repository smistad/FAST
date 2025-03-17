#include "DicomMultiFrameStreamer.hpp"
#include "dcmtk/dcmimage/diregist.h"
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dcpixel.h>
#include <dcmtk/dcmjpeg/djdecode.h>
#include <FAST/Data/Image.hpp>
#include <memory>
#include <FAST/Algorithms/Color/ColorToGrayscale.hpp>

namespace fast {

/*
template <>
std::string DicomMultiFrameStreamer::getDicomTag<std::string>(ushort group, ushort element) {
    if(!m_dataset)
        load();
    OFString patientID;
    m_dataset->findAndGetOFString(DcmTagKey(group, element),  patientID);
    return patientID.c_str();
}
*/

DicomMultiFrameStreamer::DicomMultiFrameStreamer(std::string filename, bool loop, bool useFramerate, int framerate, bool convertToGrayscale, bool cropToROI) {
    createOutputPort(0);
    m_filename = filename;
    m_convertToGrayscale = convertToGrayscale;
    m_cropToROI = cropToROI;
    m_useFramerate = useFramerate;
    setLooping(loop);
    setFramerate(framerate);
    DJDecoderRegistration::registerCodecs();// register JPEG codecs
}

void DicomMultiFrameStreamer::execute() {
    startStream();

    waitForFirstFrame();
}

void DicomMultiFrameStreamer::load() {
    if(!fileExists(m_filename))
        throw FileNotFoundException(m_filename);

    m_image = std::make_shared<DicomImage>(m_filename.c_str());
    if(m_image->getStatus() != EIS_Normal) {
        throw Exception("Error creating dicom image object");
    }
    m_fileFormat = std::make_shared<DcmFileFormat>();
    OFCondition status = m_fileFormat->loadFile(m_filename.c_str());
    if(!status.good()) {
        m_dataset = m_fileFormat->getDataset();
    } else {
        throw Exception("Unable to read dicom dataset");
    }
    if(m_image->getNumberOfFrames() == 1)
        throw Exception("Dicom file given to DicomMultiFrameStreamer is not a multi frame file.");
}

void DicomMultiFrameStreamer::generateStream() {
    if(!m_image)
        load();
    const int width = m_image->getWidth();
    const int height = m_image->getHeight();
    const int frames = m_image->getNumberOfFrames();

    Vector3f spacing = Vector3f::Ones();
    if(m_useFramerate && getFramerate() == -1) {
        // Use stored framerate
        OFString cineRate;
        OFCondition status = m_dataset->findAndGetOFString(DCM_CineRate,  cineRate, 0);
        if(status.good())
            setFramerate(std::stoi(cineRate.c_str()));
    }

    // Get pixel spacing and ROI information from ultrasound roi sequence
    DcmElement *roiSequenceElement;
    m_dataset->findAndGetElement(DCM_SequenceOfUltrasoundRegions, roiSequenceElement);

    Vector2i ROIOffset;
    Vector2i ROISize;
    bool ROIfound = false;
    if (roiSequenceElement && roiSequenceElement->ident() == EVR_SQ) {
        DcmSequenceOfItems *roiSequence = OFstatic_cast(DcmSequenceOfItems *, roiSequenceElement);

        int i = 0;
        DcmItem *roiItem = roiSequence->getItem(i);

        long int x, y, x1, y1;
        roiItem->findAndGetLongInt(DCM_RegionLocationMinX0, x);
        roiItem->findAndGetLongInt(DCM_RegionLocationMinY0, y);
        roiItem->findAndGetLongInt(DCM_RegionLocationMaxX1, x1);
        roiItem->findAndGetLongInt(DCM_RegionLocationMaxY1, y1);
        Float64 spacingX, spacingY;
        roiItem->findAndGetFloat64(DCM_PhysicalDeltaX, spacingX, 0);
        roiItem->findAndGetFloat64(DCM_PhysicalDeltaY, spacingY, 0);
        spacing.x() = spacingX;
        spacing.y() = spacingY;
        ROIOffset = Vector2i(x, y);
        ROISize = Vector2i(x1 - x, y1 - y);
        ROIfound = true;
    }

    std::cout << "size: " << width << " " << height << " " << frames <<  std::endl;

    auto previousTime = std::chrono::high_resolution_clock::now();
    while(true) {
        bool pause = getPause();
        if(pause)
            waitForUnpause();
        pause = getPause();

        {
            std::unique_lock<std::mutex> lock(m_stopMutex);
            if(m_stop) {
                m_streamIsStarted = false;
                m_firstFrameIsInserted = false;
                break;
            }
        }
        int frameNr = getCurrentFrameIndex();
        auto frameData = make_uninitialized_unique<uchar>(3*width*height);
        m_image->getOutputData(frameData.get(), 3*width*height, 0, frameNr);
        auto imageFrame = Image::create(width, height, TYPE_UINT8, 3, std::move(frameData));
        imageFrame->setSpacing(spacing);
        if(m_convertToGrayscale) {
            imageFrame = ColorToGrayscale::create()->connect(imageFrame)->runAndGetOutputData<Image>();
        }
        if(m_cropToROI && ROIfound) {
            imageFrame = imageFrame->crop(ROIOffset, ROISize);
        }
        if(!pause) {
            if(m_framerate > 0) {
                std::chrono::duration<float, std::milli> passedTime = std::chrono::high_resolution_clock::now() - previousTime;
                std::chrono::duration<int, std::milli> sleepFor(1000 / m_framerate - (int)passedTime.count());
                if(sleepFor.count() > 0)
                    std::this_thread::sleep_for(sleepFor);
                previousTime = std::chrono::high_resolution_clock::now();
            }
            getCurrentFrameIndexAndUpdate(); // Update index
        }

        try {
            addOutputData(0, imageFrame);
            frameAdded();
            if(frameNr == getNrOfFrames()) {
                throw FileNotFoundException();
            }
        } catch(FileNotFoundException &e) {
            break;
        } catch(ThreadStopped &e) {
            break;
        }
    }
}

int DicomMultiFrameStreamer::getNrOfFrames() {
    if(!m_image)
        load();
    return m_image->getNumberOfFrames();
}

DicomMultiFrameStreamer::DicomMultiFrameStreamer() {

}

DicomMultiFrameStreamer::~DicomMultiFrameStreamer() {
    DJDecoderRegistration::cleanup();
}
}