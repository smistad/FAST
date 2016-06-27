#include "FAST/Data/Image.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Algorithms/UltrasoundVesselDetection/UltrasoundVesselDetection.hpp"
#include "FAST/Algorithms/UltrasoundVesselDetection/VesselCrossSection.hpp"
#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/Algorithms/ImageCropper/ImageCropper.hpp"
#include "boost/filesystem.hpp"

using namespace fast;

inline std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d-%H%M%S", &tstruct);

    return buf;
}

int main() {
	std::string storageDir = "/home/smistad/carotis_validation_dataset/";
	boost::filesystem::create_directories(storageDir);
	// Set up stream
	std::vector<std::string> recordings = {
			/*
	"/home/smistad/AssistantTestData/1/US-Acq_03_20150608T103739/Acquisition/US-Acq_03_20150608T103739_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/1/US-Acq_04_20150608T103837/Acquisition/US-Acq_04_20150608T103837_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/1/US-Acq_07_20150608T104148/Acquisition/US-Acq_07_20150608T104148_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/1/US-Acq_08_20150608T104238/Acquisition/US-Acq_08_20150608T104238_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/0/US-Acq_03_20150608T102144/Acquisition/US-Acq_03_20150608T102144_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/0/US-Acq_04_20150608T102242/Acquisition/US-Acq_04_20150608T102242_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/0/US-Acq_07_20150608T102703/Acquisition/US-Acq_07_20150608T102703_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/0/US-Acq_08_20150608T102854/Acquisition/US-Acq_08_20150608T102854_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/2/US-Acq_03_20150608T104805/Acquisition/US-Acq_03_20150608T104805_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/2/US-Acq_04_20150608T104910/Acquisition/US-Acq_04_20150608T104910_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/2/US-Acq_07_20150608T105549/Acquisition/US-Acq_07_20150608T105549_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/2/US-Acq_08_20150608T105649/Acquisition/US-Acq_08_20150608T105649_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/3/US-Acq_03_20150608T113646/Acquisition/US-Acq_03_20150608T113646_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/3/US-Acq_04_20150608T113750/Acquisition/US-Acq_04_20150608T113750_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/3/US-Acq_07_20150608T114115/Acquisition/US-Acq_07_20150608T114115_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/3/US-Acq_08_20150608T114217/Acquisition/US-Acq_08_20150608T114217_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/4/US-Acq_03_20150608T112610/Acquisition/US-Acq_03_20150608T112610_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/4/US-Acq_04_20150608T112656/Acquisition/US-Acq_04_20150608T112656_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/4/US-Acq_08_20150608T113129/Acquisition/US-Acq_08_20150608T113129_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/4/US-Acq_09_20150608T113245/Acquisition/US-Acq_09_20150608T113245_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/5/US-Acq_03_20150608T111610/Acquisition/US-Acq_03_20150608T111610_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/5/US-Acq_04_20150608T111701/Acquisition/US-Acq_04_20150608T111701_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/5/US-Acq_07_20150608T111940/Acquisition/US-Acq_07_20150608T111940_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/6/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/6/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/6/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/6/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/6/4/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/7/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/7/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/7/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/7/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/8/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/8/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/8/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/8/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/9/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/9/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/9/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/9/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/10/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/10/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/10/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/10/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/11/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/11/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/11/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/11/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/12/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/12/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/12/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/12/3/US-2D_#.mhd",
	*/
			/*
	"/media/smistad/New\ Volume/Assistant\ Recordings/Leuven\ 5\ feb\ 2016/2016-02-05-105012/US-2D_#.mhd",
	"/media/smistad/New\ Volume/Assistant\ Recordings/Leuven\ 5\ feb\ 2016/2016-02-05-105257/US-2D_#.mhd",
	"/media/smistad/New\ Volume/Assistant\ Recordings/Leuven\ 6\ feb\ 2016/2016-02-06-094808/US-2D_#.mhd",
	"/media/smistad/New\ Volume/Assistant\ Recordings/Leuven\ 6\ feb\ 2016/2016-02-06-095237/US-2D_#.mhd",

	"/media/smistad/New\ Volume/Assistant\ Recordings/Leuven\ 5\ feb\ 2016/2016-02-05-114458/US-2D_#.mhd",
	"/media/smistad/New\ Volume/Assistant\ Recordings/Leuven\ 5\ feb\ 2016/2016-02-05-120411/US-2D_#.mhd",
	"/media/smistad/New\ Volume/Assistant\ Recordings/Leuven\ 5\ feb\ 2016/2016-02-05-120648/US-2D_#.mhd",
	"/media/smistad/New\ Volume/Assistant\ Recordings/Leuven\ 5\ feb\ 2016/2016-02-05-120843/US-2D_#.mhd"
	*/
			"/media/smistad/New Volume/Carotis/2016-06-02-140622/US-2D_#.mhd",
			"/media/smistad/New Volume/Carotis/2016-06-02-140721/US-2D_#.mhd",
			"/media/smistad/New Volume/Carotis/2016-06-02-140846/US-2D_#.mhd",
			"/media/smistad/New Volume/Carotis/2016-06-02-140923/US-2D_#.mhd"
	};
	for(int i = 0; i < recordings.size(); ++i) {
		ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
		streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
		streamer->setStepSize(20);
		streamer->setFilenameFormat(recordings[i]);
		streamer->update();
		DynamicData::pointer images = streamer->getOutputData<Image>();

		// Put in subfolders
		int j = i / 4; // Group 4 sequences into each folder
		std::string targetDir = storageDir + boost::lexical_cast<std::string>(j) + "/";
		boost::filesystem::create_directories(targetDir);

		UltrasoundVesselDetection::pointer dummy = UltrasoundVesselDetection::New();

		// Store ROIs as PNGs to disk
		int counter = 0;
		UltrasoundVesselDetection::pointer detector = UltrasoundVesselDetection::New();
		while(!images->hasReachedEnd()) {
			Image::pointer image = images->getNextFrame(dummy);
			Vector3ui imageSize = image->getSize();
			Vector3f spacing = image->getSpacing();

			detector->setInputData(image);
			detector->update();
			std::vector<VesselCrossSection::pointer> crossSections = detector->getCrossSections();
			std::cout << "Found " << crossSections.size() << " cross sections" << std::endl;

			// For each detected black ellipse
			for(VesselCrossSection::pointer crossSection : crossSections) {
				VesselCrossSectionAccess::pointer access = crossSection->getAccess(ACCESS_READ);
				Vector2f imageCenter = access->getImageCenterPosition();

				// Radius in pixels
				const float majorRadius = access->getMajorRadius();
				const float minorRadius = access->getMinorRadius();
				const int frameSize = std::max((int)round(majorRadius), 50); // Nr if pixels to include around vessel

				Vector2i offset(
						round(imageCenter.x() - majorRadius) - frameSize,
						round(imageCenter.y() - majorRadius) - frameSize
				);
				int size2 = 2*majorRadius + 2*frameSize;
				Vector2i size(
						size2,
						size2
				);

				ImageCropper::pointer cropper = ImageCropper::New();
				cropper->setInputData(image);
				cropper->allowOutOfBoundsCropping(true);
				cropper->setOffset(offset);
				cropper->setSize(size);

				ImageExporter::pointer exporter = ImageExporter::New();
				std::string filename = currentDateTime() + "-" + boost::lexical_cast<std::string>(counter) + ".png";
				exporter->setFilename(targetDir + filename);
				exporter->setInputConnection(cropper->getOutputPort());
				exporter->update();
				counter++;
			}
		}
		std::cout << "Finished recording " << (i+1) << " of " << recordings.size() << std::endl;
	}
}
