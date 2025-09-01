#include "ClariusStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include <cast/cast.h>
#include <functional>
#include <QGLContext>
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

    loadLibrary();
}

void ClariusStreamer::loadLibrary() {

    // Load Clarius Cast library dynamically
    reportInfo() << "Loading clarius cast library" << reportEnd();
#ifdef WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS); // To avoid diaglog box, when not able to load a DLL
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

ClariusStreamer::pointer ClariusStreamer::self = nullptr; // class static

void ClariusStreamer::execute() {
    if(!mStreamIsStarted) {
#ifndef WIN32
        // If a GL context is currently active, we have to disable it when initializing clarius cast,
        // or it will crash, most likely cast is using OpenGL as well.
        QGLContext* glContext = (QGLContext*)QGLContext::currentContext();
        if(glContext != nullptr)
            glContext->doneCurrent();
#endif
        reportInfo() << "Trying to set up Clarius streaming..." << reportEnd();
        std::string keydir = Config::getKernelBinaryPath();
        // TODO A hack here to get this to work. Fix later
        // Lambdas converted to C style pointers can't have captures
        self = std::dynamic_pointer_cast<ClariusStreamer>(mPtr.lock());

        CusInitParams params;
        params.args.argc = 0;
        params.storeDir = keydir.c_str();
        // All callbacks have to be defined, or it will crash
        params.newProcessedImageFn = [](const void* img, const CusProcessedImageInfo* nfo, int npos, const CusPosInfo* pos) {
            self->newImageFn(img, nfo, npos, pos);
        };
        params.buttonFn = [](CusButton btn, int clicks) {
            self->getReporter().info() << "button pressed: " << (btn == CusButton::ButtonDown ? "DOWN" : "UP") << self->getReporter().end();
        };
        params.newRawImageFn = [](const void* img, const CusRawImageInfo* nfo, int npos, const CusPosInfo* pos) {
            self->getReporter().info() << "raw image received" << self->getReporter().end();
        };
        params.newImuDataFn = [](const CusPosInfo* pos) {
            self->getReporter().info() << "imu data received" << self->getReporter().end();
        };
        params.progressFn = [](int progress) {
            self->getReporter().info() << "progress: " << progress  << self->getReporter().end();
        };
        params.newSpectralImageFn = [](const void* img, const CusSpectralImageInfo* nfo) {
           self->getReporter().info() << "spectral image received" << self->getReporter().end();
        };
        params.freezeFn = [](int state) {
            self->getReporter().info() << "Freeze " << ((state == 1) ? "ON" : "OFF") << self->getReporter().end();
        };
        params.width = 512;
        params.height = 512;
        params.errorFn = [](const char* msg) {
			self->signalCastStop(msg);
        };

        auto init = (int (*)(const CusInitParams* params))getFunc("cusCastInit");
        int success = init(&params);
        if(success != 0)
            throw Exception("Unable to initialize clarius cast");
        reportInfo() << "Clarius streamer initialized" << reportEnd();

        mStreamIsStarted = true;
        m_thread = std::make_unique<std::thread>(std::bind(&ClariusStreamer::generateStream, this));

        auto connect = (int (*)(const char* ipAddress, unsigned int port, const char* cert, CusConnectFn fn))getFunc("cusCastConnect");
        success = connect(mIPAddress.c_str(), mPort, "research", [](int port, int imuPort, int swMatch) {
			if (port > 0) {
				self->getReporter().info() << "Clarius connect on UDP port " << port << self->getReporter().end();
				if (swMatch == CUS_FAILURE) {
					self->getReporter().warning() << "A software mismatch between Clarius Cast API in FAST and the scanner was detected." << self->getReporter().end();
				}
			}
			else {
                self->signalCastStop("Failed to connect to Clarius device");
			}
        });
        if(success != 0)
            throw Exception("Unable to connect to clarius scanner");
        reportInfo() << "Clarius streamer connecting ...." << reportEnd();
#ifndef WIN32
        if(glContext != nullptr)
            glContext->makeCurrent();
#endif
    }

    // Wait here for first frame
    waitForFirstFrame();
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
    frameAdded();
    mNrOfFrames++;
}

uint ClariusStreamer::getNrOfFrames() {
    return mNrOfFrames;
}

ClariusStreamer::~ClariusStreamer() {
    reportInfo() << "Destroying ClariuStreamer ..." << reportEnd();
    stop();
			
    // Calling destroy in the cast thread causes crash:
    // Calling destroy in generateStream causes block
    // TODO Check if Cast exist?
    auto destroy = (int (*)())getFunc("cusCastDestroy");
    getReporter().info() << "Destroying cast ..." << getReporter().end();
    int success = destroy();
    getReporter().info() << "Clarius Destroy done." << getReporter().end();
    if (success < 0)
        getReporter().error() << "Unable to destroy clarius cast" << getReporter().end();

    reportInfo() << "ClariuStreamer destroyed" << reportEnd();
}

void ClariusStreamer::generateStream() {
    // Use this thread to listen to a stop signal
    std::unique_lock<std::mutex> lock(m_castStopMutex);
    while(!m_castStop) {
        m_castStopCV.wait(lock);
    }
    stop();
	stopWithError(m_stopMessage); // This will cause ThreadStopped exception
    reportInfo() << "ClariusStreamer::generateStream() done" << reportEnd();
}

void ClariusStreamer::signalCastStop(std::string message) {
    {
        std::lock_guard<std::mutex> lock(m_castStopMutex);
        m_castStop = true;
        m_stopMessage = message;
    }
    m_castStopCV.notify_all();
}

void ClariusStreamer::stop() {
    reportInfo() << "In clarius streamer stop()" << reportEnd();
    if(!mStreamIsStarted)
        return;
    reportInfo() << "Stopping clarius streamer.." << reportEnd();
    mStreamIsStarted = false;
    auto disconnect = (int (*)(CusReturnFn))getFunc("cusCastDisconnect");
    int success = disconnect([](int code) {
        self->getReporter().info() << "Clarius Disconnect callback with return code: " << ((code == CUS_FAILURE) ? "Failure" : "Success") << self->getReporter().end();
		self.reset(); // When is it safe to do this?
    });
    reportInfo() << "Clarius Disconnect done." << reportEnd();
    if(success == 0)
        reportError() << "Unable to disconnect from clarius scanner" << reportEnd();
	frameAdded(); // Unblock execute if needed
    reportInfo() << "Clarius streamer stopped" << Reporter::end();
    // TODO How to stop program gracefully if no frames are ever streamed?
}

void ClariusStreamer::stopPipeline() {
    signalCastStop();
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
    auto userFunc = (int (*)(CusUserFunction cmd, double val, CusReturnFn fn))getFunc("cusCastUserFunction");
    if(userFunc(Freeze, 0.0, nullptr) < 0)
        reportError() << "Error toggling freeze" << reportEnd();
}

void ClariusStreamer::increaseDepth() {
    auto userFunc = (int (*)(CusUserFunction cmd, double val, CusReturnFn fn))getFunc("cusCastUserFunction");
	if(userFunc(DepthInc, 0.0, nullptr) < 0)
        reportError() << "Error increasing depth" << reportEnd();
}

void ClariusStreamer::decreaseDepth() {
    auto userFunc = (int (*)(CusUserFunction cmd, double val, CusReturnFn fn))getFunc("cusCastUserFunction");
	if(userFunc(DepthDec, 0.0, nullptr) < 0)
        reportError() << "Error decreasing depth" << reportEnd();
}

void ClariusStreamer::setDepth(float depth) {
    auto userFunc = (int (*)(CusUserFunction cmd, double val, CusReturnFn fn))getFunc("cusCastUserFunction");
	if(userFunc(SetDepth, depth, nullptr) < 0)
        reportError() << "Error setting depth to " << depth << reportEnd();
}

void ClariusStreamer::setGain(float gain) {
    auto userFunc = (int (*)(CusUserFunction cmd, double val, CusReturnFn fn))getFunc("cusCastUserFunction");
	if(userFunc(SetGain, gain, nullptr) < 0)
        reportError() << "Error setting gain to " << gain << reportEnd();
}

}
