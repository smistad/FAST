#ifndef SEEDEDREGIONGROWING_HPP_
#define SEEDEDREGIONGROWING_HPP_

#include "ProcessObject.hpp"
#include "Image.hpp"
#include "DataTypes.hpp"

namespace fast {

class SeededRegionGrowing : public ProcessObject {
    FAST_OBJECT(SeededRegionGrowing)
    public:
        void setInput(ImageData::pointer input);
        void setIntensityRange(float min, float max);
        void addSeedPoint(uint x, uint y);
        void addSeedPoint(uint x, uint y, uint z);
        void addSeedPoint(Vector3ui position);
        ImageData::pointer getOutput();
    private:
        SeededRegionGrowing();
        void execute();
        void waitToFinish();
        void recompileOpenCLCode(Image::pointer input);
        template <class T>
        void executeOnHost(T* input, Image::pointer output);

        float mMinimumIntensity, mMaximumIntensity;
        std::vector<Vector3ui> mSeedPoints;

        cl::Kernel mKernel;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;

};

} // end namespace fast



#endif /* SEEDEDREGIONGROWING_HPP_ */
