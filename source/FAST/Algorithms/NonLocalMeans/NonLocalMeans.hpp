#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {
    class FAST_EXPORT NonLocalMeans : public ProcessObject {
        FAST_PROCESS_OBJECT(NonLocalMeans);
    public:
        FAST_CONSTRUCTOR13(NonLocalMeans, int, filterSize, = 3, int, searchSize, = 11, float, smoothingAmount, = 0.15, int, multiScaleIterations, = 3)
        void setSmoothingAmount(float parameterH);
        void setPreProcess(bool preProcess);
        void setMultiscaleIterations(int iterations);
        void setSearchSize(int searchSize);
        void setFilterSize(int filterSize);
        void loadAttributes() override;
    private:
        void init();
        void execute() override;

        float m_parameterH = 0.15f;
        bool m_preProcess = true;
        int m_iterations = 3; // How many multiscale iterations to do
        int m_searchSize = 11; // How large the pixel search area should be
        int m_filterSize = 3;
    };
}