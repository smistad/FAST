#ifndef MULTIGRID_GRADIENT_VECTOR_FLOW_HPP_
#define MULTIGRID_GRADIENT_VECTOR_FLOW_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

class FAST_EXPORT  MultigridGradientVectorFlow : public ProcessObject {
    FAST_OBJECT(MultigridGradientVectorFlow)
    public:
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
        MultigridGradientVectorFlow();
        void execute();
        void execute3DGVF(SharedPointer<Image> input, SharedPointer<Image> output, uint iterations);

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

#endif
