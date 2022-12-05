#include "ClariusStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include <cast/cast.h>
#include <functional>
#ifdef WIN32
#else
#include <dlfcn.h>
#endif

namespace fast {

#ifdef WIN32
//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
static std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}
#endif



ClariusStreamer::ClariusStreamer(std::string ipAddress, int port, bool grayscale) {
    createOutputPort<Image>(0);
    mNrOfFrames = 0;
    mFirstFrameIsInserted = false;
    mStreamIsStarted = false;
    mIsModified = true;
    mGrayscale = grayscale;

    createStringAttribute("ip", "IP address", "IP address of Clarius device to connect to", ipAddress);
    createIntegerAttribute("port", "Port number", "Port number of Clarius device to connect to", port);
    createBooleanAttribute("grayscale", "Grayscale", "Convert input image to grayscale", mGrayscale);

    setConnectionAddress(ipAddress);
    setConnectionPort(port);
    setStreamingMode(StreamingMode::NewestFrameOnly);

    // Load Clarius Cast library dynamically
    reportInfo() << "Loading clarius cast library" << reportEnd();
#ifdef WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS); // TODO To avoid diaglog box, when not able to load a DLL
    m_handle = LoadLibrary("cast.dll");
    if(!m_handle) {
        std::string msg = GetLastErrorAsString();
        throw Exception("Failed to use load Clarius Cast library. Error message: " + msg);
    }
#elif defined(__APPLE__)
    m_handle = dlopen("libcast.dylib", RTLD_LAZY);
    if(!m_handle) {
        std::string msg = dlerror();
        throw Exception("Failed to use load Clarius Cast library. Error message: " + msg);
    }
#else
    m_handle = dlopen("libcast.so", RTLD_LAZY);
    if(!m_handle) {
        std::string msg = dlerror();
        throw Exception("Failed to use load Clarius Cast library. Note that this only supports Ubuntu 20.04, see https://github.com/clariusdev/cast. Error message: " + msg);
    }
#endif
    reportInfo() << "Finished loading clarius cast library" << reportEnd();
}

void ClariusStreamer::loadAttributes() {
    mIPAddress = getStringAttribute("ip");
    mPort = getIntegerAttribute("port");
    mGrayscale = getBooleanAttribute("grayscale");
}

void* ClariusStreamer::getFunc(std::string name) {
#ifdef WIN32
    auto func = GetProcAddress(m_handle, name.c_str());
    if(!func) {
        std::string msg = GetLastErrorAsString();
        FreeLibrary(m_handle);
        throw Exception("Failed to get address of function in ClariusStreamer. Error message: " + msg);
    }
    return func;
#else
    auto func = dlsym(m_handle, name.c_str());
    if(!func) {
        std::string msg = dlerror();
        dlclose(m_handle);
        throw Exception("Failed to get address of function in ClariusStreamer. Error message: " + msg);
    }
    return func;
#endif
}

void ClariusStreamer::execute() {
    if(!mStreamIsStarted) {
        reportInfo() << "Trying to set up Clarius streaming..." << reportEnd();
        int argc = 0;
        std::string keydir = Config::getKernelBinaryPath();
        // TODO A hack here to get this to work. Fix later
        static ClariusStreamer::pointer self = std::dynamic_pointer_cast<ClariusStreamer>(mPtr.lock());

        auto init = (int (*)(int argc,
                char** argv,
                const char* dir,
                CusNewProcessedImageFn newProcessedImage,
                CusNewRawImageFn newRawImage,
                CusNewSpectralImageFn newSpectralImage,
                CusFreezeFn freeze,
                CusButtonFn btn,
                CusProgressFn progress,
                CusErrorFn err,
                int width,
                int height
                ))getFunc("cusCastInit");
        int success = init(argc, nullptr, keydir.c_str(),
            // new image callback
            [](const void* img, const CusProcessedImageInfo* nfo, int npos, const CusPosInfo* pos)
            {
                self->newImageFn(img, nfo, npos, pos);
            },
            nullptr/*pre-scanconverted image*/, 
            nullptr/*spectral image*/, 
		    nullptr/*freeze*/, 
		    nullptr/*button*/, 
		    nullptr/*progress*/, 
			/*error call back*/
			[](const char* msg) {
                self->getReporter().error() << msg << self->getReporter().end();
            },
			512, 
			512
		);
        if(success < 0)
            throw Exception("Unable to initialize clarius cast");
        reportInfo() << "Clarius streamer initialized" << reportEnd();

        auto connect = (int (*)(const char* ipAddress, unsigned int port, const char* cert, CusReturnFn fn))getFunc("cusCastConnect");
        success = connect(mIPAddress.c_str(), mPort, "research", nullptr);
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

void ClariusStreamer::newImageFn(const void *img, const _CusProcessedImageInfo *nfo, int npos,
                                 const _CusPosInfo *pos) {
    if(nfo->bitsPerPixel != 32)
        throw Exception("Expected 32 bits per pixel (4 channels with 8 bits) each in ClariusStreamer, but got " + std::to_string(nfo->bitsPerPixel));

    // Copy pixels
    Image::pointer image;
    const int width = nfo->width;
    const int height = nfo->height;
    if(mGrayscale) {
        const auto img2 = static_cast<const uchar*>(img);
        auto pixels = make_uninitialized_unique<uchar[]>(width * height);
        for(int i = 0; i < width*height; ++i) {
            pixels[i] = img2[i * 4 + 0];
        }
        image = Image::create(width, height, TYPE_UINT8, 1, std::move(pixels));
    } else {
        image = Image::create(width, height, TYPE_UINT8, 4, img);
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
    auto disconnect = (int (*)(CusReturnFn))getFunc("cusCastDisconnect");
    int success = disconnect(nullptr);
    if(success < 0)
        throw Exception("Unable to disconnect from clarius scanner");
    auto destroy = (int (*)())getFunc("cusCastDestroy");
    success = destroy();
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
    auto userFunc = (int (*)(int cmd, double val, CusReturnFn fn))getFunc("cusCastUserFunction");
    if(userFunc(Freeze, 0.0, nullptr) < 0)
        reportError() << "Error toggling freeze" << reportEnd();
}

void ClariusStreamer::increaseDepth() {
    auto userFunc = (int (*)(int cmd, double val, CusReturnFn fn))getFunc("cusCastUserFunction");
	if(userFunc(DepthInc, 0.0, nullptr) < 0)
        reportError() << "Error increasing depth" << reportEnd();
}

void ClariusStreamer::decreaseDepth() {
    auto userFunc = (int (*)(int cmd, double val, CusReturnFn fn))getFunc("cusCastUserFunction");
	if(userFunc(DepthDec, 0.0, nullptr) < 0)
        reportError() << "Error decreasing depth" << reportEnd();
}

void ClariusStreamer::setDepth(float depth) {
    auto userFunc = (int (*)(int cmd, double val, CusReturnFn fn))getFunc("cusCastUserFunction");
	if(userFunc(SetDepth, depth, nullptr) < 0)
        reportError() << "Error setting depth to " << depth << reportEnd();
}

void ClariusStreamer::setGain(float gain) {
    auto userFunc = (int (*)(int cmd, double val, CusReturnFn fn))getFunc("cusCastUserFunction");
	if(userFunc(SetGain, gain, nullptr) < 0)
        reportError() << "Error setting gain to " << gain << reportEnd();
}

}
