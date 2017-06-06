#ifndef REPORT_HPP_
#define REPORT_HPP_

#include <map>
#include <iostream>
#include "FASTExport.hpp"

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
};

template <class T>
void Reporter::process(const T& content) {
    Method reportMethod = getMethod(mType);

    if(mFirst) {
        // Write prefix first
        if(reportMethod == COUT) {
            if(mType == INFO) {
                std::cout << "INFO: ";
            } else if(mType == WARNING) {
                std::cout << "WARNING: ";
            } else if(mType == ERROR) {
                std::cout << "ERROR: ";
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

#endif
