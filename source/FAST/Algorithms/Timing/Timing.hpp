#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT StartTimer : public ProcessObject {
    FAST_PROCESS_OBJECT(StartTimer)
    public:
        FAST_CONSTRUCTOR(StartTimer)
        std::chrono::system_clock::time_point getStartTime() const;
    private:
        void execute() override;

        std::chrono::system_clock::time_point m_startTime;
};

class FAST_EXPORT StopTimer : public ProcessObject {
    FAST_PROCESS_OBJECT(StopTimer)
    public:
        FAST_CONSTRUCTOR(StopTimer,
                         std::shared_ptr<StartTimer>, start,,
                         bool, printEveryRun, = true,
                         bool, printAtEnd, = true,
                         int, warmupRounds, = 0,
                         int, maximumSamples, = -1
        )
        void setStartTimer(StartTimer::pointer start);
        float getFPS() const;
        float getAverage() const;
        RuntimeMeasurement::pointer getRuntime() const;
        ~StopTimer();
    private:
        StopTimer();
        void execute() override;

        bool m_printEveryRun = true;
        bool m_printAtEnd = true;
        std::shared_ptr<RuntimeMeasurement> m_measurement;
        std::shared_ptr<StartTimer> m_startTimer;
};

}