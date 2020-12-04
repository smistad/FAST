#include "ClariusStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include <cast/cast.h>
#include <functional>

namespace fast {


ClariusStreamer::ClariusStreamer() {
	Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    createOutputPort<Image>(0);
    mNrOfFrames = 0;
    mFirstFrameIsInserted = false;
    mStreamIsStarted = false;
    mIsModified = true;
    mGrayscale = true;

    createStringAttribute("ip", "IP address", "IP address of Clarius device to connect to", mIPAddress);
    createIntegerAttribute("port", "Port number", "Port number of Clarius device to connect to", mPort);
    createBooleanAttribute("grayscale", "Grayscale", "Convert input image to grayscale", mGrayscale);
}

void ClariusStreamer::loadAttributes() {
    mIPAddress = getStringAttribute("ip");
    mPort = getIntegerAttribute("port");
    mGrayscale = getBooleanAttribute("grayscale");
}

void ClariusStreamer::execute() {
    if(!mStreamIsStarted) {
        reportInfo() << "Trying to set up Clarius streaming..." << reportEnd();
        int argc = 0;
        std::string keydir = Config::getKernelBinaryPath();
        // TODO A hack here to get this to work. Fix later
        static ClariusStreamer::pointer self = std::dynamic_pointer_cast<ClariusStreamer>(mPtr.lock());
        int success = clariusInitCast(argc, nullptr, keydir.c_str(),
            // new image callback
            [](const void* img, const ClariusProcessedImageInfo* nfo, int npos, const ClariusPosInfo* pos)
            {
                self->newImageFn(img, nfo, npos, pos);
            },
            nullptr/*pre-scanconverted image*/, 
		    nullptr/*freeze*/, 
		    nullptr/*button*/, 
		    nullptr/*progress*/, 
			/*error call back*/
			[](const char* msg) { 
                self->getReporter().error() << msg << self->getReporter().end(); 
            }, 
			nullptr/*return callback*/, 
			512, 
			512
		);
        if(success < 0)
            throw Exception("Unable to initialize clarius cast");
        reportInfo() << "Clarius streamer initialized" << reportEnd();

        success = clariusConnect(mIPAddress.c_str(), mPort, nullptr);
        if(success < 0)
            throw Exception("Unable to connect to clarius scanner");
        reportInfo() << "Clarius streamer connected." << reportEnd();
        mStreamIsStarted = true;
    }

    // Wait here for first frame
    std::unique_lock<std::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
}

void ClariusStreamer::newImageFn(const void *img, const _ClariusProcessedImageInfo *nfo, int npos,
                                 const _ClariusPosInfo *pos) {
    if(nfo->bitsPerPixel != 32)
        throw Exception("Expected 32 bits per pixel (4 channels with 8 bits) each in ClariusStreamer, but got " + std::to_string(nfo->bitsPerPixel));

    // Copy pixels
    auto image = Image::New();
    const int width = nfo->width;
    const int height = nfo->height;
    if(mGrayscale) {
        const auto img2 = static_cast<const uchar*>(img);
        auto pixels = make_uninitialized_unique<uchar[]>(width * height);
        for(int i = 0; i < width*height; ++i) {
            pixels[i] = img2[i * 4 + 0];
        }
        image->create(width, height, TYPE_UINT8, 1, std::move(pixels));
    } else {
        image->create(width, height, TYPE_UINT8, 4, img);
    }
    float spacing = (float)nfo->micronsPerPixel/1000.0f; // convert spacing to millimeters
    image->setSpacing(Vector3f(spacing, spacing, 1));
    image->setCreationTimestamp(nfo->tm / 1000000); // convert timestsamp to milliseconds

    addOutputData(0, image);
    if(!mFirstFrameIsInserted) {
    {
        std::lock_guard<std::mutex> lock(mFirstFrameMutex);
        mFirstFrameIsInserted = true;
    }
    mFirstFrameCondition.notify_one();
    }
    mNrOfFrames++;
}

uint ClariusStreamer::getNrOfFrames() {
    return mNrOfFrames;
}

ClariusStreamer::~ClariusStreamer() {
    stop();
}

void ClariusStreamer::stop() {
    int success = clariusDisconnect(nullptr);
    if(success < 0)
        throw Exception("Unable to disconnect from clarius scanner");
    success = clariusDestroyCast();
    if(success < 0)
        throw Exception("Unable to destroy clarius cast");

    reportInfo() << "Clarius streamer stopped" << Reporter::end();
}

void ClariusStreamer::setConnectionAddress(std::string ipAddress) {
    mIPAddress = ipAddress;
}

void ClariusStreamer::setConnectionPort(int port) {
    if(port <= 0)
        throw Exception("Illegal port nr " + std::to_string(port));
    mPort = port;
}

void ClariusStreamer::toggleFreeze() {
    if(clariusUserFunction(USER_FN_TOGGLE_FREEZE, 0.0, nullptr) < 0)
        reportError() << "Error toggling freeze" << reportEnd();
}

void ClariusStreamer::increaseDepth() {
	if(clariusUserFunction(USER_FN_DEPTH_INC, 0.0, nullptr) < 0)
        reportError() << "Error increasing depth" << reportEnd();
}

void ClariusStreamer::decreaseDepth() {
	if(clariusUserFunction(USER_FN_DEPTH_DEC, 0.0, nullptr) < 0)
        reportError() << "Error decreasing depth" << reportEnd();
}

void ClariusStreamer::setDepth(float depth) {
	if(clariusUserFunction(USER_FN_SET_DEPTH, depth, nullptr) < 0)
        reportError() << "Error setting depth to " << depth << reportEnd();
}

void ClariusStreamer::setGain(float gain) {
	if(clariusUserFunction(USER_FN_SET_GAIN, gain, nullptr) < 0)
        reportError() << "Error setting gain to " << gain << reportEnd();
}

}
