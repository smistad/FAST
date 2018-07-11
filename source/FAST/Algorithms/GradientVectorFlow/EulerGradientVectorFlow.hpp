#ifndef EULER_GRADIENT_VECTOR_FLOW_HPP_
#define EULER_GRADIENT_VECTOR_FLOW_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

class FAST_EXPORT  EulerGradientVectorFlow : public ProcessObject {
    FAST_OBJECT(EulerGradientVectorFlow)
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
        EulerGradientVectorFlow();
        void execute();
        void execute2DGVF(SharedPointer<Image> input, SharedPointer<Image> output, uint iterations);
        void execute3DGVF(SharedPointer<Image> input, SharedPointer<Image> output, uint iterations);
        void execute3DGVFNo3DWrite(SharedPointer<Image> input, SharedPointer<Image> output, uint iterations);

        float mMu;
        uint mIterations;
        bool mUse16bitFormat;
};

} // end namespace fast

#endif
