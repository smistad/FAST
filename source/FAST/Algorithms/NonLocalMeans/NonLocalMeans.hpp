#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {
/**
 * @brief Multiscale Non-Local Means (NLM) smoothing
 *
 * Non-Local Means (NLM) is an excellent despeckling filter for ultrasound images.
 * This GPU implementation is based on the article "Real-Time Nonlocal Means-Based Despeckling" by Breivik et al. 2017.
 *
 * @ingroup filter
 */
class FAST_EXPORT NonLocalMeans : public ProcessObject {
    FAST_PROCESS_OBJECT(NonLocalMeans);
    public:
        /**
         * @brief Creates instance of this process object
         * @param filterSize Size in pixels of the filter region to search for. Must be odd.
         * @param searchSize How many pixels to search in each direction. Must be odd.
         * @param smoothingAmount Parameter to control the amount of smoothing.
         * @param multiScaleIterations Number of multiscale iterations to perform
         * @param inputMultiplicationWeight If > 0, the input image will be multiplied with the output with this weight.
         * @return smart pointer to instance
         */
        FAST_CONSTRUCTOR(NonLocalMeans,
                         int, filterSize, = 3,
                         int, searchSize, = 11,
                         float, smoothingAmount, = 0.15f,
                         float, inputMultiplicationWeight, = 0.8f,
                         int, multiScaleIterations, = 3
        );
        void setSmoothingAmount(float parameterH);
        void setPreProcess(bool preProcess);
        void setMultiscaleIterations(int iterations);
        void setSearchSize(int searchSize);
        void setFilterSize(int filterSize);
        void setInputMultiplicationWeight(float weight);
        void loadAttributes() override;
    private:
        void init();
        void execute() override;

        float m_parameterH = 0.15f;
        bool m_preProcess = true;
        int m_iterations = 3; // How many multiscale iterations to do
        int m_searchSize = 11; // How large the pixel search area should be
        int m_filterSize = 3;
        float m_multiplicationWeight = 0.5f;
    };
}