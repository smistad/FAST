#pragma once
#include <FAST/Visualization/Plotting/Plotter.hpp>

namespace fast {

FAST_SIMPLE_DATA_OBJECT(FloatScalar, float);
FAST_SIMPLE_DATA_OBJECT(FloatPoint, Vector2f);

// Input data objects: simple data object: float.., create one line per input connection
// Redraw whenever new data arrives
class FAST_EXPORT LinePlotter : public Plotter {
    FAST_PROCESS_OBJECT(LinePlotter);
    Q_OBJECT
public:
    FAST_CONSTRUCTOR(LinePlotter, int, bufferSize, = 64)
    uint addInputConnection(DataChannel::pointer channel, std::string name = "");
    //uint addInputConnection(DataChannel::pointer channel, std::string name, Color color);
    void setBufferSize(int size);
    void addHorizontalLine(float x, Color color = Color::Green());
public slots:
    void processQueue();
protected:
    void execute() override;
    int m_bufferSize = 64;
    std::map<uint, std::vector<double>> m_buffer;
    std::mutex m_queueMutex;
    std::deque<std::map<int, Vector2f>> m_queue;
    std::vector<double> m_xAxis;
    int m_currentIndex = 0;
    std::map<uint, std::string> m_names;
    std::uint64_t m_frameCounter = 0;
};

}
