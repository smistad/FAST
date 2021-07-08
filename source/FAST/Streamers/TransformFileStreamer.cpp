#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Utility.hpp"
#include "TransformFileStreamer.hpp"
#include <FAST/Data/DataBoundingBox.hpp>
#include <fstream>
#include <chrono>

namespace fast {

TransformFileStreamer::TransformFileStreamer() {
    mIsModified = true;
    mLoop = false;
    mTimestampFilename = "";
    mSleepTime = 0;
    mNrOfFrames = 0;
    createOutputPort<Transform>(0);
}

TransformFileStreamer::TransformFileStreamer(std::string filename, std::string timestampFilename) : TransformFileStreamer() {
    setFilename(filename);
    setTimestampFilename(timestampFilename);
}

void TransformFileStreamer::setSleepTime(uint milliseconds) {
    mSleepTime = milliseconds;
}

void TransformFileStreamer::setTimestampFilename(std::string filepath) {
    mTimestampFilename = filepath;
}

void TransformFileStreamer::execute() {
    if(mFilename == "")
        throw Exception("No filename was given to the TransformFileStreamer");

    startStream();

    waitForFirstFrame();
}

void TransformFileStreamer::setFilename(std::string str) {
    mFilename = str;
}

void TransformFileStreamer::generateStream() {
    // Read timestamp file if available
    std::ifstream timestampFile;
    unsigned long previousTimestamp = 0;
    auto previousTimestampTime = std::chrono::high_resolution_clock::time_point::min();
    if(mTimestampFilename != "") {
        timestampFile.open(mTimestampFilename.c_str());
        if(!timestampFile.is_open()) {
            throw Exception("Timestamp file not found in TransformFileStreamer");
        }
    }

    // Open file
    std::ifstream transformationFile(mFilename);
    if(!transformationFile.is_open()) {
    	throw Exception("Transformation file " + mFilename + " not found in TransformFileStreamer");
    }

    while(true) {
		// Read the next transformation from the file into a matrix
		Matrix4f matrix = Matrix4f::Identity();
		std::string line;
		std::vector<std::string> elements;

		bool restart = false;
		for(int row = 0; row < 3; ++row) {
			std::getline(transformationFile, line);
			elements = split(line);

			trim(elements[0]);
			if(elements.size() != 4 && elements.size() > 1) {
				throw Exception("Error reading transformation file " + mFilename + " expected 4 numbers per line, "
						+ std::to_string(elements.size()) + " found.");
			} else if(elements.size() == 1 && elements[0] == "") {
				reportInfo() << "Reached end of stream" << Reporter::end();
				// If there where no files found at all, we need to release the execute method
				frameAdded();
				if(mLoop) {
					// Restart stream
					if(timestampFile.is_open()) {
						previousTimestamp = 0;
						previousTimestampTime = std::chrono::high_resolution_clock::time_point::min();
						timestampFile.clear(); // Must clear errors before reset
						timestampFile.seekg(0); // Reset file to start
					}
					transformationFile.clear(); // Must clear errors before reset
					transformationFile.seekg(0); // Reset file to start
					restart = true;
				}
				// Reached end of stream
				break;
			}

			matrix.row(row) = Vector4f(
					std::stof(elements[0]),
					std::stof(elements[1]),
					std::stof(elements[2]),
					std::stof(elements[3]));
		}

		if(restart)
			continue;

		auto transformation = Transform::create();
		transformation->get().matrix() = matrix;

		// Set and use timestamp if available
		if(mTimestampFilename != "") {
			std::string line;
			std::getline(timestampFile, line);
			if(line != "") {
				unsigned long timestamp = std::stoul(line);
				transformation->setCreationTimestamp(timestamp);
				// Wait as long as necessary before adding image
				auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::high_resolution_clock::now() - previousTimestampTime);
				//reportInfo() << timestamp << reportEnd();
				//reportInfo() << previousTimestamp << reportEnd();
				//reportInfo() << "Time passed: " << timePassed.count() << reportEnd();
				while(timestamp > previousTimestamp + timePassed.count()) {
					// Wait
					std::this_thread::sleep_for(std::chrono::milliseconds(timestamp-(long)previousTimestamp-timePassed.count()));
					timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::high_resolution_clock::now() - previousTimestampTime);
					//reportInfo() << "wait" << reportEnd();
					//reportInfo() << timestamp << reportEnd();
					//reportInfo() << previousTimestamp << reportEnd();
					//reportInfo() << "Time passed: " << timePassed.count() << reportEnd();
				}
				previousTimestamp = timestamp;
				previousTimestampTime = std::chrono::high_resolution_clock::now();
			}
		}
        addOutputData(0, transformation);
        frameAdded();
        if(mSleepTime > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTime));
		mNrOfFrames++;
    }
}


uint TransformFileStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}

TransformFileStreamer::~TransformFileStreamer() {
    stop();
}

void TransformFileStreamer::enableLooping() {
    mLoop = true;
}

void TransformFileStreamer::disableLooping() {
    mLoop = false;
}

} // end namespace fast
