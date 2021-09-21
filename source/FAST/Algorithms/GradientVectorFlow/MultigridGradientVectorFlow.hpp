#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

/**
 * @brief Gradient vector flow using the multigrid method
 *
 * This is only implemented for 3D.
 * Gradient vector flow is a spatial diffusion of vectors often used for segmentation.
 * This 3D GPU implementation is described in the article "Multigrid gradient vector flow computation on the GPU"
 * by Smistad et. al 2014: https://www.eriksmistad.no/wp-content/uploads/multigrid_gradient_vector_flow_computation_on_the_gpu.pdf
 *
 * @ingroup segmentation
 */
class FAST_EXPORT  MultigridGradientVectorFlow : public ProcessObject {
    FAST_PROCESS_OBJECT(MultigridGradientVectorFlow)
    public:
        /**
         * @brief Create instance
         * @param mu
         * @param iterations
         * @param use16BitStorage
         * @return instance
         */
        FAST_CONSTRUCTOR(MultigridGradientVectorFlow,
                         float, mu, = 0.1f,
                         uint, iterations, = 10,
                         bool, use16BitStorage, = true
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
        void execute3DGVF(std::shared_ptr<Image> input, std::shared_ptr<Image> output, uint iterations);

        float mMu;
        uint mIterations;
        bool mUse16bitFormat;
        cl::Program mProgram;

        cl::Image3D initSolutionToZero(Vector3ui size, int imageType, int bufferSize);
        void gaussSeidelSmoothing(
            cl::Image3D &v,
            cl::Image3D &r,
            cl::Image3D &sqrMag,
            int iterations,
            Vector3ui size,
            float mu,
            float spacing,
            int imageType,
            int bufferSize
        );
        cl::Image3D restrictVolume(
        cl::Image3D &v,
        Vector3ui newSize,
        int imageType,
        int bufferSize
        );
        cl::Image3D prolongateVolume(
        cl::Image3D &v_l,
        cl::Image3D &v_l_p1,
        Vector3ui size,
        int imageType,
        int bufferSize
        );
        cl::Image3D prolongateVolume2(
        cl::Image3D &v_l_p1,
        Vector3ui size,
        int imageType,
        int bufferSize
        );
        cl::Image3D residual(
        cl::Image3D &r,
        cl::Image3D &v,
        cl::Image3D &sqrMag,
        float mu,
        float spacing,
        Vector3ui size,
        int imageType,
        int bufferSize
        );
        void multigridVcycle(
        cl::Image3D &r_l,
        cl::Image3D &v_l,
        cl::Image3D &sqrMag,
        int l,
        int v1,
        int v2,
        int l_max,
        float mu,
        float spacing,
        Vector3ui size,
        int imageType,
        int bufferSize
        );
        cl::Image3D computeNewResidual(
        cl::Image3D &f,
        cl::Image3D &vectorField,
        float mu,
        float spacing,
        int component,
        Vector3ui size,
        int imageType,
        int bufferSize
        );
        cl::Image3D fullMultigrid(
        cl::Image3D &r_l,
        cl::Image3D &sqrMag,
        int l,
        int v0,
        int v1,
        int v2,
        int l_max,
        float mu,
        float spacing,
        Vector3ui size,
        int imageType,
        int bufferSize
        );
};

} // end namespace fast
