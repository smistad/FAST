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
