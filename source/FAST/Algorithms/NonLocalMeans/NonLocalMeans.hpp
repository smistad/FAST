#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {
    class FAST_EXPORT NonLocalMeans : public ProcessObject {
        FAST_OBJECT(NonLocalMeans);
    public:
        void setSmoothingAmount(float parameterH);
        void setPreProcess(bool preProcess);
        void setMultiscaleIterations(int iterations);
        void setSearchSize(int searchSize);
        void setFilterSize(int filterSize);
    private:
        NonLocalMeans();
        void execute() override;

        float m_parameterH = 0.15f;
        bool m_preProcess = true;
        int m_iterations = 3; // How many multiscale iterations to do
        int m_searchSize = 11; // How large the pixel search area should be
        int m_filterSize = 3;
    };
}