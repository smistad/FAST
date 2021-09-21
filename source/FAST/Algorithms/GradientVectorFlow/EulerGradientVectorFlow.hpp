#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

/**
 * @brief Gradient vector flow using Euler method
 *
 * Gradient vector flow is a spatial diffusion of vectors often used for segmentation.
 * This 2D/3D GPU implementation is described in the article "Real-time gradient vector flow on GPUs using OpenCL"
 * by Smistad et. al 2015: https://www.eriksmistad.no/wp-content/uploads/gpu_gradient_vector_flow_opencl.pdf
 *
 * @ingroup segmentation
 */
class FAST_EXPORT EulerGradientVectorFlow : public ProcessObject {
    FAST_PROCESS_OBJECT(EulerGradientVectorFlow)
    public:
        /**
         * @brief Create instance
         * @param mu
         * @param iterations
         * @param use16bitStorage
         * @return
         */
        FAST_CONSTRUCTOR(EulerGradientVectorFlow,
                         float, mu, = 0.05f,
                         uint, iterations, = 0,
                         bool, use16bitStorage, = true
        );
        void setIterations(uint iterations);
        void setMuConstant(float mu);
        float getMuConstant() const;
        /**
         * Use 16 bit format internally to reduce memory usage and
         * increase performance.
         * This will slightly reduce accuracy/convergence.
         */
        void set16bitStorageFormat();
        /**
         * Use 32 bit format internally instead of 16 bit.
         */
        void set32bitStorageFormat();
    private:
        void execute();
        void execute2DGVF(std::shared_ptr<Image> input, std::shared_ptr<Image> output, uint iterations);
        void execute3DGVF(std::shared_ptr<Image> input, std::shared_ptr<Image> output, uint iterations);
        void execute3DGVFNo3DWrite(std::shared_ptr<Image> input, std::shared_ptr<Image> output, uint iterations);

        float mMu;
        uint mIterations;
        bool mUse16bitFormat;
};

} // end namespace fast
