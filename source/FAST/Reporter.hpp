#pragma once

#include <map>
#include <iostream>
#include <thread>
#include "FASTExport.hpp"
#ifdef WIN32
#include <windows.h>
#endif

#undef ERROR // undefine some windows garbage

namespace fast {

// Use to signal end of report line
class FAST_EXPORT  ReporterEnd {

};

class FAST_EXPORT  Reporter {
    public:
        static ReporterEnd end();
        static Reporter info();
        static Reporter warning();
        static Reporter error();
        enum Type {INFO, WARNING, ERROR};
        enum Method {NONE, COUT, LOG};
        void setType(Type);
        Reporter(Type type);
        Reporter();
        template <class T>
        void process(const T& content);
        void processEnd();
        void setReportMethod(Method method);
        void setReportMethod(Type type, Method method);
        static void setGlobalReportMethod(Method method);
        static void setGlobalReportMethod(Type type, Method method);
    private:
        Method getMethod(Type) const;
        Type mType;
        static std::map<Type, Method> mGlobalReporterMethods;
        // The local report methods override the global, if they are defined
        std::map<Type, Method> mLocalReporterMethods;

        // Variable to keep track of first <<
        bool mFirst;
#ifdef WIN32
        static WORD m_defaultAttributes;
#endif
};

template <class T>
void Reporter::process(const T& content) {
    Method reportMethod = getMethod(mType);

    if(mFirst) {
        // Write prefix first
        if(reportMethod == COUT) {
            if(mType == INFO) {
                std::cout << "INFO [" << std::this_thread::get_id() << "] ";
            } else if(mType == WARNING) {
#ifdef WIN32
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (m_defaultAttributes & 0x00F0) | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#else
                std::cout << "\033[1m";
#endif
                std::cout << "WARNING [" << std::this_thread::get_id() << "] ";
            } else if(mType == ERROR) {
#ifdef WIN32
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (m_defaultAttributes & 0x00F0) | FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
                std::cout << "\033[31;1m";
#endif
                std::cerr << "ERROR [" << std::this_thread::get_id() << "] ";
            }
        }
        mFirst = false;
    }

    if(reportMethod == COUT) {
        std::cout << content;
    } else if(reportMethod == LOG) {
        // Not implemented yet
    }
}

template <class T>
Reporter operator<<(Reporter report, const T& content) {
    report.process(content);
    return report;
}

template <>
FAST_EXPORT Reporter operator<<(Reporter report, const ReporterEnd& end);

} // end namespace fast
