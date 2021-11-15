#include "Timing.hpp"

namespace fast {

StartTimer::StartTimer() {
    createInputPort(0);
    createOutputPort(0);
}

void StartTimer::execute() {
    auto input = getInputData<DataObject>();
    m_startTime = std::chrono::system_clock::now();
    addOutputData(input);
}

std::chrono::system_clock::time_point StartTimer::getStartTime() const {
    return m_startTime;
}

StopTimer::StopTimer() {
    createInputPort(0);
    createOutputPort(0);
    m_measurement = std::make_shared<RuntimeMeasurement>("");
}

StopTimer::StopTimer(StartTimer::pointer start, bool printEveryRun, bool printAtEnd, int warmupRounds, int maximumSmaples) : StopTimer() {
    m_measurement = std::make_shared<RuntimeMeasurement>("", warmupRounds, maximumSmaples);
    setStartTimer(start);
    m_printAtEnd = printAtEnd;
    m_printEveryRun = printEveryRun;
}

void StopTimer::execute() {
    auto input = getInputData<DataObject>();

    std::chrono::duration<double, std::milli> time = std::chrono::system_clock::now() - m_startTimer->getStartTime();
    m_measurement->addSample(time.count());
    if(m_printEveryRun)
        m_measurement->print();

    addOutputData(input);
}

StopTimer::~StopTimer() noexcept {
    if(m_printAtEnd)
        m_measurement->print();
}

RuntimeMeasurement::pointer StopTimer::getRuntime() const {
    return m_measurement;
}

float StopTimer::getAverage() const {
    return m_measurement->getAverage();
}

float StopTimer::getFPS() const {
    return 1000.0f / m_measurement->getAverage();
}

void StopTimer::setStartTimer(StartTimer::pointer start) {
    m_startTimer = start;
    setModified(true);
}



}