#include "FAST/Object.hpp"
#include <iostream>
#include <mutex>

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
#ifdef WIN32
    CONSOLE_SCREEN_BUFFER_INFO Info;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hStdout, &Info);
    auto defaultAttributes = Info.wAttributes;
#endif
    // This terminate handler basically just writes out the what() error message of unhandled exceptions
    try {
        auto exception = std::current_exception();
        if(exception) {
            std::rethrow_exception(exception);
        } else {
            // Normal termination, do nothing
        }
    } catch (const std::exception & e) { // For exceptions based on std::exception
#ifdef WIN32
        SetConsoleTextAttribute(hStdout, (defaultAttributes & 0x00F0) | FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
        std::cerr << "\033[31;1m";
#endif
        std::cerr << "ERROR [" << std::this_thread::get_id() << "] Terminated with unhandled exception: " << e.what() << std::endl;
#ifdef WIN32
        SetConsoleTextAttribute(hStdout, defaultAttributes);
#else
        std::cerr << "\033[0m";
#endif
    } catch (...) { // For other things like `throw 1;`
#ifdef WIN32
        SetConsoleTextAttribute(hStdout, (defaultAttributes & 0x00F0) | FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
        std::cerr << "\033[31;1m";
#endif
        std::cerr << "ERROR [" << std::this_thread::get_id() << "] Terminated with unknown exception." << std::endl;
#ifdef WIN32
        SetConsoleTextAttribute(hStdout, defaultAttributes);
#else
        std::cerr << "\033[0m";
#endif
    }
}

Object::Object() {
    if (std::get_terminate() != terminateHandler) {
        // Terminate handler not set, create it:
        std::set_terminate(terminateHandler);
    }

    static std::once_flag flag;
    std::call_once(flag, []() {
        // Print the splash
        // TODO Add config option to disable splash
#ifdef WIN32
        CONSOLE_SCREEN_BUFFER_INFO Info;
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hStdout, &Info);
        auto defaultAttributes = Info.wAttributes;
        SetConsoleTextAttribute(hStdout, (defaultAttributes & 0x00F0) | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#else
        std::cout << "\033[32;1m"; // Green bold
#endif
        std::cout << "\n     - Powered by -     \n"
            "   _______   __________   \n"
            "  / __/ _ | / __/_  __/   \n"
            " / _// __ |_\\ \\  / /    https://fast.eriksmistad.no\n"
            "/_/ /_/ |_/___/ /_/       \n\n";
#if WIN32
        SetConsoleTextAttribute(hStdout, defaultAttributes);
#else
        std::cout << "\033[0m"; // Reset
#endif
    });
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
