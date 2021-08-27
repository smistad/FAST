#include "Reporter.hpp"

namespace fast {

// Initialize report methods for each type
std::map<Reporter::Type, Reporter::Method> Reporter::mGlobalReporterMethods =
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
#ifdef WIN32
WORD Reporter::m_defaultAttributes = 0;
#endif

Reporter::Reporter(Type type) {
    mType = type;
    mFirst = true;
#ifdef WIN32
    if(m_defaultAttributes == 0) {
        CONSOLE_SCREEN_BUFFER_INFO Info;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &Info);
        m_defaultAttributes = Info.wAttributes;
    }
#endif
}

Reporter::Reporter() {
    mType = INFO;
    mFirst = true;
#ifdef WIN32
    if(m_defaultAttributes == 0) {
        CONSOLE_SCREEN_BUFFER_INFO Info;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &Info);
        m_defaultAttributes = Info.wAttributes;
    }
#endif
}

void Reporter::setType(Type type) {
    mType = type;
}

Reporter Reporter::info() {
    return Reporter(INFO);
}

Reporter Reporter::warning() {
    return Reporter(WARNING);
}

Reporter Reporter::error() {
    return Reporter(ERROR);
}

void Reporter::processEnd() {
    mFirst = true;
    if(getMethod(mType) == COUT) {
#if WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_defaultAttributes);
        std::cout << std::endl;
#else
        std::cout << "\033[0m" << std::endl; // Reset
#endif
    } else if(getMethod(mType) == LOG) {
        // Not implemented yet
    }
}

void Reporter::setReportMethod(Method method)  {
    mLocalReporterMethods[INFO] = method;
    mLocalReporterMethods[WARNING] = method;
    mLocalReporterMethods[ERROR] = method;
}

void Reporter::setReportMethod(Type type, Method method)  {
    mLocalReporterMethods[type] = method;
}

void Reporter::setGlobalReportMethod(Method method)  {
    mGlobalReporterMethods[INFO] = method;
    mGlobalReporterMethods[WARNING] = method;
    mGlobalReporterMethods[ERROR] = method;
}

void Reporter::setGlobalReportMethod(Type type, Method method)  {
    mGlobalReporterMethods[type] = method;
}

Reporter::Method Reporter::getMethod(Type type) const {
    Method reportMethod;
    // If a local report method is given for the type, use that, if not use the global
    if(mLocalReporterMethods.count(type) > 0) {
        reportMethod = mLocalReporterMethods.at(type);
    } else {
        reportMethod = mGlobalReporterMethods.at(type);
    }
    return reportMethod;
}

template <>
Reporter operator<<(Reporter report, const ReporterEnd& end) {
    report.processEnd();
    return report;
}

ReporterEnd Reporter::end() {
    return ReporterEnd();

}

Reporter::Method Reporter::getGlobalReportMethod(Type type) {
    return mGlobalReporterMethods[type];
}

} // end namespace fast
