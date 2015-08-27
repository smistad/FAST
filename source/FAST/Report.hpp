#ifndef REPORT_HPP_
#define REPORT_HPP_

#include <map>
#include <iostream>

#undef ERROR // undefine some windows garbage

namespace fast {

// Use to signal end of report line
class ReportEnd {

};

class Report {
    public:
        static ReportEnd end;
        static Report info() {
            return Report(INFO);
        }
        static Report warning() {
            return Report(WARNING);
        }
        static Report error() {
            return Report(ERROR);
        }
        enum Type {INFO, WARNING, ERROR};
        enum Method {NONE, COUT, LOG};
        Report(Type type);
        template <class T>
        void process(const T& content);
        void processEnd();
        static void setReportMethod(Method method);
        static void setReportMethod(Type type, Method method);
    private:
        Type mType;
        static std::map<Type, Method> mReportMethods;
};

template <class T>
void Report::process(const T& content) {
    if(mReportMethods[mType] == COUT) {
        std::cout << content;
    } else if(mReportMethods[mType] == LOG) {
        // Not implemented yet
    }
}

template <class T>
Report operator<<(Report report, const T& content) {
    report.process(content);
    return report;
}

template <>
Report operator<<(Report report, const ReportEnd& end);

} // end namespace fast

#endif
