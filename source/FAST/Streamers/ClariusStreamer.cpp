#include "ClariusStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include <listen/listen.h>
#include <functional>

namespace fast {


ClariusStreamer::ClariusStreamer() {
    createOutputPort<Image>(0);
    mNrOfFrames = 0;
    mHasReachedEnd = false;
    mFirstFrameIsInserted = false;
    mStreamIsStarted = false;
    mIsModified = true;
    mGrayscale = false;
}

void ClariusStreamer::execute() {
    if(!mStreamIsStarted) {
        reportInfo() << "Trying to set up Clarius streaming..." << reportEnd();
        int argc = 0;
        std::string keydir = Config::getKernelBinaryPath();
        // TODO A hack here to get this to work. Fix later
        static ClariusStreamer::pointer self = std::dynamic_pointer_cast<ClariusStreamer>(mPtr.lock());
        int success = clariusInitListener(argc, nullptr, keydir.c_str(),
                // new image callback
                [](const void* img, const ClariusImageInfo* nfo, int npos, const ClariusPosInfo* pos)
              {
                self->newImageFn(img, nfo, npos, pos);
              },
              nullptr, nullptr, nullptr, nullptr);
        if(success < 0)
            throw Exception("Unable to initialize clarius listener");
        reportInfo() << "Clarius streamer initialized" << reportEnd();

        std::string ipAddr = "192.168.1.1";
        uint port = 5828;
        success = clariusConnect(ipAddr.c_str(), port, nullptr);
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

void ClariusStreamer::newImageFn(const void *img, const _ClariusImageInfo *nfo, int npos,
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

bool ClariusStreamer::hasReachedEnd() {
    return mHasReachedEnd;
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

    success = clariusDestroyListener();
    if(success < 0)
        throw Exception("Unable to destroy clarius listener");

    reportInfo() << "Clarius streamer stopped" << Reporter::end();
}

}
