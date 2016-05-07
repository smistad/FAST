#include "FAST/Data/Image.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Algorithms/UltrasoundVesselDetection/UltrasoundVesselDetection.hpp"
#include "FAST/Algorithms/UltrasoundVesselDetection/VesselCrossSection.hpp"
#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/Algorithms/ImageCropper/ImageCropper.hpp"

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
	const int frameSize = 40; // Nr if pixels to include around vessel
	std::string storageDir = "/home/smistad/vessel_detection_dataset/";
	// Set up stream
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	//streamer->setFilenameFormat("/home/smistad/ES/US-Acq_03_20150608T103739/Acquisition/US-Acq_03_20150608T103739_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/ES/US-Acq_04_20150608T103837/Acquisition/US-Acq_04_20150608T103837_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/ES/US-Acq_07_20150608T104148/Acquisition/US-Acq_07_20150608T104148_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/ES/US-Acq_08_20150608T104238/Acquisition/US-Acq_08_20150608T104238_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/DHI/US-Acq_04_20150608T104910/Acquisition/US-Acq_04_20150608T104910_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/IR/US-Acq_04_20150608T112656/Acquisition/US-Acq_04_20150608T112656_Image_Transducer_#.mhd");
	streamer->setFilenameFormat("/home/smistad/AssistantTestData/DHI/US-Acq_04_20150608T104910/Acquisition/US-Acq_04_20150608T104910_Image_Transducer_#.mhd");
	streamer->setStepSize(10);
	streamer->update();
	DynamicData::pointer images = streamer->getOutputData<Image>();


	UltrasoundVesselDetection::pointer dummy = UltrasoundVesselDetection::New();

	// Store ROIs as PNGs to disk
	int counter = 0;
	while(!images->hasReachedEnd()) {
		Image::pointer image = images->getNextFrame(dummy);
		Vector3ui imageSize = image->getSize();
		Vector3f spacing = image->getSpacing();

		ImageExporter::pointer exporter = ImageExporter::New();
		std::string filename = currentDateTime() + "-" + boost::lexical_cast<std::string>(counter) + "-original.png";
		exporter->setFilename(storageDir + filename);
		exporter->setInputData(image);
		exporter->update();
		counter++;
		UltrasoundVesselDetection::pointer detector = UltrasoundVesselDetection::New();
		detector->setInputData(image);
		detector->update();
		std::vector<VesselCrossSection::pointer> crossSections = detector->getCrossSections();
		std::cout << "Found " << crossSections.size() << " cross sections" << std::endl;

		// For each detected black ellipse
		for(VesselCrossSection::pointer crossSection : crossSections) {
			VesselCrossSectionAccess::pointer access = crossSection->getAccess(ACCESS_READ);
			Vector2f imageCenter = access->getImageCenterPosition();

			// Radius in pixels
			float majorRadius = access->getMajorRadius();
			float minorRadius = access->getMinorRadius();

			Vector2i offset(
					round(imageCenter.x() - majorRadius) - frameSize,
					round(imageCenter.y() - minorRadius) - frameSize
			);
			Vector2ui size(
					2*majorRadius + 2*frameSize,
					2*minorRadius + 2*frameSize
			);

			// Clamp to image bounds
			if(offset.x() < 0)
				offset.x() = 0;
			if(offset.y() < 0)
				offset.y() = 0;
			if(offset.x() + size.x() > imageSize.x())
				size.x() = imageSize.x() - offset.x();
			if(offset.y() + size.y() > imageSize.y())
				size.y() = imageSize.y() - offset.y();

			ImageCropper::pointer cropper = ImageCropper::New();
			cropper->setInputData(image);
			cropper->setOffset(offset.cast<uint>());
			cropper->setSize(size);

			ImageExporter::pointer exporter = ImageExporter::New();
			std::string filename = currentDateTime() + "-" + boost::lexical_cast<std::string>(counter) + ".png";
			exporter->setFilename(storageDir + filename);
			exporter->setInputConnection(cropper->getOutputPort());
			exporter->update();
			counter++;
		}
	}
}
