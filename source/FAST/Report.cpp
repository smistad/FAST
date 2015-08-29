#include "Report.hpp"

namespace fast {

// Initialize report methods for each type
std::map<Report::Type, Report::Method> Report::mReportMethods =
#ifdef FAST_DEBUG
{
        {INFO, COUT},
        {WARNING, COUT},
        {ERROR, COUT}
};
#else
{
        {INFO, NONE},
        {WARNING, COUT},
        {ERROR, COUT}
};
#endif
ReportEnd Report::end = ReportEnd();

Report::Report(Type type) {
    mType = type;
    // Add prefix
    if(type == INFO && mReportMethods[INFO] == COUT) {
        std::cout << "INFO: ";
    } else if(type == WARNING && mReportMethods[WARNING] == COUT) {
        std::cout << "WARNING: ";
    } else if(type == ERROR && mReportMethods[ERROR] == COUT) {
        std::cout << "ERROR: ";
    }
}

void Report::processEnd() {
    if(mReportMethods[mType] == COUT) {
        std::cout << std::endl;
    } else if(mReportMethods[mType] == LOG) {
        // Not implemented yet
    }
}

void Report::setReportMethod(Method method)  {
    mReportMethods[INFO] = method;
    mReportMethods[WARNING] = method;
    mReportMethods[ERROR] = method;
}

void Report::setReportMethod(Type type, Method method)  {
    mReportMethods[type] = method;
}

template <>
Report operator<<(Report report, const ReportEnd& end) {
    report.processEnd();
    return report;
}

} // end namespace fast
