#pragma once

#include <FAST/Streamers/RandomAccessStreamer.hpp>

class DicomImage;
class DcmDataset;
class DcmFileFormat;

namespace fast {

/**
 * @brief Stream images from a dicom multi-frame image
 *
 * <h3>Output ports</h3>
 * - 0: Image
 *
 * @ingroup streamers ultrasound
 */
class FAST_EXPORT DicomMultiFrameStreamer : public RandomAccessStreamer {
    FAST_PROCESS_OBJECT(DicomMultiFrameStreamer)
    public:
        /**
         * @brief Create instance
         * @param filename Dicom file to open
         * @param loop Whether to loop or not
         * @param useFramerate Whether to use framerate from dicom file or not. If this is set to false, images will be streamed as fast as possible
         * @param framerate If framerate is > 0, this framerate will be used for streaming the images
         * @param grayscale Convert images to grayscale
         * @param cropToROI Try to extract ROI from dicom and crop the images to this ROI
         * @return instance
         */
        FAST_CONSTRUCTOR(DicomMultiFrameStreamer,
                         std::string, filename,,
                         bool, loop, = false,
                         bool, useFramerate, = true,
                         int, framerate, = -1,
                         bool, grayscale, = false,
                         bool, cropToROI, = false
         );
        int getNrOfFrames() override;
        //template <class T>
        //T getDicomTag(ushort group, ushort element);
        ~DicomMultiFrameStreamer();
    private:
        DicomMultiFrameStreamer();
        void execute();
        void generateStream() override;
        void load();

        bool m_useFramerate = true;
        bool m_convertToGrayscale = false;
        bool m_cropToROI = false;
        std::string m_filename;
        std::shared_ptr<DicomImage> m_image;
        std::shared_ptr<DcmFileFormat> m_fileFormat;
        DcmDataset* m_dataset; // owned by DcmFileFormat
};

//template<>
//std::string DicomMultiFrameStreamer::getDicomTag<std::string>(ushort group, ushort element);

}