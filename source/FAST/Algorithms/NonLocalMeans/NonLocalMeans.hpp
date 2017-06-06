#ifndef NONELOCALMEANS_HPP_
#define NONELOCALMEANS_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class FAST_EXPORT  NonLocalMeans : public ProcessObject {
	FAST_OBJECT(NonLocalMeans)
	public:
		//void setSigma(unsigned char s);
        void setSigma(float s);
        void setK(char k);
        void setGroupSize(char gS);
		void setWindowSize(char wS);
		void setDenoiseStrength(float dS);
		void setOutputType(DataType type);
        void setEuclid(char e);
        float getSigma();
        int getK();
        int getGroupSize();
        int getWindowSize();
        float getDenoiseStrength();
		void waitToFinish();
	private:
		NonLocalMeans();
		void execute();
		void recompileOpenCLCode(Image::pointer input);

		unsigned char windowSize;
		unsigned char groupSize;
        unsigned char k;
        unsigned char euclid;
		//unsigned char sigma;
        float sigma;
        float denoiseStrength;

		//bool gray;
		//bool mIsModified;
		bool recompile;

		cl::Kernel mKernel;
		unsigned char mDimensionCLCodeCompiledFor;
		DataType mTypeCLCodeCompiledFor;
		DataType mOutputType;
		bool mOutputTypeSet;


}; //emd class
} //end namespace

#endif //NONELOCALMEANS_HPP_
