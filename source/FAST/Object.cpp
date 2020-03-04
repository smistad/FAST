#include "FAST/Object.hpp"
#include <iostream>

#if defined(__APPLE__) || defined(__MACOSX)

#else
#if _WIN32
#include <windows.h>

#else
#include <GL/glx.h>
#endif
#endif

#undef ERROR // undefine some windows garbage

namespace fast {

static void terminateHandler() {
    // This terminate handler basically just writes out the what() error message of unhandled exceptions
    try {
        auto exception = std::current_exception();
        if(exception) {
            std::rethrow_exception(exception);
        } else {
            // Normal termination, do nothing
        }
    } catch (const std::exception & e) { // For exceptions based on std::exception
        std::cerr << "ERROR: Terminated with unhandled exception: " << e.what() << std::endl;
    } catch (...) { // For other things like `throw 1;`
        std::cerr << "ERROR: Terminated with unknown exception." << std::endl;
    }
}

Object::Object() {
    if(std::get_terminate() != terminateHandler) {
        // Terminate handler not set, create it:
        std::set_terminate(terminateHandler);
    }
}

Reporter& Object::reportError() {
    mReporter.setType(Reporter::ERROR);
    return mReporter;
}

Reporter& Object::reportWarning() {
    mReporter.setType(Reporter::WARNING);
    return mReporter;
}

Reporter& Object::reportInfo() {
    mReporter.setType(Reporter::INFO);
    return mReporter;
}

Reporter& Object::getReporter() {
    return mReporter;
}

ReporterEnd Object::reportEnd() const {
    return Reporter::end();
}

} // end namespace fast
