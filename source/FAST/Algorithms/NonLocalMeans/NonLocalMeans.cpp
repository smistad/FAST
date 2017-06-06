#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
#include <numeric>

#include "NonLocalMeans.hpp"
using namespace fast;

NonLocalMeans::NonLocalMeans() {
	createInputPort<Image>(0);
	createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NonLocalMeans/NonLocalMeans2Dgs.cl", "2D");
    //createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NonLocalMeans/NonLocalMeans2Dgaussian.cl", "2Dg");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NonLocalMeans/NonLocalMeans3Dgs.cl", "3D");
	windowSize = 11;
	groupSize = 3;
	sigma = 0.3f;
	denoiseStrength = 0.15f;
	mDimensionCLCodeCompiledFor = 0;
    k = 0;
    euclid = 0;
	mOutputTypeSet = false;
    mIsModified = true;
    recompile = true;
}

void NonLocalMeans::setOutputType(DataType type){
	mOutputType = type;
	mOutputTypeSet = true;
	mIsModified = true;
    recompile = true;
}
void NonLocalMeans::setK(char newK){
    if (newK < 0){
        throw Exception("NoneLocalMeans K must be >= 0.");
    }
    k = newK;
    mIsModified = true;
    recompile = true;
}

void NonLocalMeans::setEuclid(char e){
    if (e < 0){
        throw Exception("NoneLocalMeans Euclid must be >= 0.");
    }
    e = euclid;
    mIsModified = true;
    recompile = true;
}

void NonLocalMeans::setWindowSize(char wS) {
	if (wS <= 0)
		throw Exception("NoneLocalMeans window size must be greater then 0.");
	if (wS % 2 != 1)
		throw Exception("NoneLocalMeans window size must be odd.");

	windowSize = wS;
    mIsModified = true;
	recompile = true;
}

void NonLocalMeans::setGroupSize(char gS) {
	if (gS <= 0)
		throw Exception("NoneLocalMeans group size must be greater then 0.");
	if (gS % 2 != 1)
		throw Exception("NoneLocalMeans group size must be odd.");

	groupSize = gS;
    mIsModified = true;
	recompile = true;
}

void NonLocalMeans::setDenoiseStrength(float dS){
	if (dS <= 0)
		throw Exception("NoneLocalMeans denoise strength must be greater then 0.");

    std::cout << "CHANGING DENOISE TO: " << dS << std::endl;
	denoiseStrength = dS;
    mIsModified = true;
	//recompile = true;
}

void NonLocalMeans::setSigma(float s){
    if (s <= 0)
        throw Exception("NoneLocalMeans sigma must be greater then 0.");
    
    sigma = s;
    mIsModified = true;
    recompile = true;
}



template <class T>
void executeAlgorithmOnHost(Image::pointer input, Image::pointer output, unsigned char group, unsigned char window, float strength, unsigned char sigma) {
	throw Exception("This is on host, does not work atm");
    
    ImageAccess::pointer inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess::pointer outputAccess = output->getImageAccess(ACCESS_READ_WRITE);

    T * inputData = (T*)inputAccess->get();
    T * outputData = (T*)outputAccess->get();

    unsigned int width = input->getWidth();
    unsigned int height = input->getHeight();
    //Window is window-1/2
    //group is group-1/2
    //strength is strength*strength
    //sigma is sigma*sigma
    //Not working atm with the T
    //Does not work with outofbounds atm
    //So this code is for all pixels inbound, meaning x + group + window < width / x - group - window > 0 //same for y
    for (int x = 0; x < width; x++){
        for (int y = 0; y < height; y++){
            
            double normSum = 0.0;
            double totSum = 0.0;
            double indi = 0.0;
            double groupTot = 0.0;
            double value = 0.0;

            for (int i = x - window; i <= x + window; i++){
                for (int j = y - window; j <= y + window; j++){
                    if (i != x && j != y){

                        int mX = x - group;
                        int mY = y - group;
                        for (int k = i - group; k <= i + group; k++, mX++){
                            for (int l = j - group; l <= j + group; l++, mY++){
                                //This is wrong, need to fix T
                                //indi = inputData[mX][mY] - inputData[k][l];
                                indi = fabs(indi*indi);
                                indi = exp( - (indi/strength));
                                groupTot += indi;
                            }
                        }
                        //This is wrong, need to fix T
                        //value = inputData[i][j];
                        double pA[] = {(double)i,(double)j};
                        double pB[] = {(double)x,(double)y};
                        //double dist = i, j - x, y;
                        double dist = std::inner_product(std::begin(pA), std::end(pA), std::begin(pB), 0.0);
                        
                        double gaussWeight = exp(-(dist / (2.0 * sigma)));
                        gaussWeight = gaussWeight / (2.0 * sigma);
                        groupTot *= gaussWeight;

                        normSum += groupTot;
                        totSum += groupTot * value;
                        groupTot = 0.0;
                    }
                }
            }
            value = totSum / normSum;
            /*
            Not sure it needed
            if (value < 0){
                value = 0;
            }
            if (value > 1.0){
                value = 1.0f;
            }
            */
            //This is wrong, need to fix T
            //outputData[x][y] = (T)value;
            
        }
    }
}

void NonLocalMeans::recompileOpenCLCode(Image::pointer input) {
    // Check if there is a need to recompile OpenCL code
    if(input->getDimensions() == mDimensionCLCodeCompiledFor &&
       input->getDataType() == mTypeCLCodeCompiledFor && !recompile)
        return;

    std::cout << "RECOMPILING..." << std::endl;
    recompile = false;
    OpenCLDevice::pointer device = getMainDevice();
    std::string buildOptions = "";
    if(!device->isWritingTo3DTexturesSupported()) {
        buildOptions = "-DTYPE=" + getCTypeAsString(mOutputType);
    }
    buildOptions += " -D WINDOW=";
    buildOptions += std::to_string((windowSize-1)/2);
    buildOptions += " -D GROUP=";
    buildOptions += std::to_string((groupSize-1)/2);
    buildOptions += " -D KVERSION=";
    buildOptions += std::to_string(k);
    buildOptions += " -D EUCLID=";
    buildOptions += std::to_string(euclid);
    cl::Program program;
    if(input->getDimensions() == 2) {
        program = getOpenCLProgram(device, "2D", buildOptions);
    } else {
        //createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NoneLocalMeans/NoneLocalMeans3Dgs.cl", "3D");
        program = getOpenCLProgram(device, "3D", buildOptions);
    }
    mKernel = cl::Kernel(program, "noneLocalMeans");
    mDimensionCLCodeCompiledFor = input->getDimensions();
    mTypeCLCodeCompiledFor = input->getDataType();
}
/*
void NoneLocalMeans::recompileOpenCLCode(Image::pointer input) {
	// Check if there is a need to recompile OpenCL code
	if (input->getDimensions() == mDimensionCLCodeCompiledFor &&
		input->getDataType() == mTypeCLCodeCompiledFor && !recompile)
		return;

	OpenCLDevice::pointer device = getMainDevice();
    recompile = false;
	std::string buildOptions = "";
	const bool writingTo3DTextures = device->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos;
	if (!writingTo3DTextures) {
		switch (mOutputType) {
		case TYPE_FLOAT:
			buildOptions += " -DTYPE=float";
			break;
		case TYPE_INT8:
			buildOptions += " -DTYPE=char";
			break;
		case TYPE_UINT8:
			buildOptions += " -DTYPE=uchar";
			break;
		case TYPE_INT16:
			buildOptions += " -DTYPE=short";
			break;
		case TYPE_UINT16:
			buildOptions += " -DTYPE=ushort";
			break;
		}
	}
    buildOptions += " -D WINDOW=";
    buildOptions += std::to_string((windowSize-1)/2);
	buildOptions += " -D GROUP=";
    buildOptions += std::to_string((groupSize-1)/2);
    
	std::string filename;
	//might have to seperate color vs gray here, for better runtime
	if (input->getDimensions() == 2) {
        if(k == 0){
            filename = "Algorithms/NoneLocalMeans/NoneLocalMeans2Dconstant.cl";
        }else if(k == 1){
            filename = "Algorithms/NoneLocalMeans/NoneLocalMeans2Dgaussian.cl";
        }else{
            filename = "Algorithms/NoneLocalMeans/NoneLocalMeans2Dconstant.cl";
        }
		//filename = "Algorithms/NoneLocalMeans/NoneLocalMeans2DgsPixelWise.cl";
		//filename = "Algorithms/NoneLocalMeans/NoneLocalMeans2Dgs.cl";
        //filename = "Algorithms/NoneLocalMeans/NoneLocalMeans2Dc.cl";
	}
	else {
		filename = "Algorithms/NoneLocalMeans/NoneLocalMeans3Dgs.cl";
	}
	int programNr = device->createProgramFromSource(Config::getKernelSourcePath() + filename, buildOptions);
	mKernel = cl::Kernel(device->getProgram(programNr), "noneLocalMeans");
	mDimensionCLCodeCompiledFor = input->getDimensions();
	mTypeCLCodeCompiledFor = input->getDataType();
}*/
void NonLocalMeans::execute() {
    std::cout << "EXECUTE" << std::endl;
    Image::pointer input = getStaticInputData<Image>(0);
    Image::pointer output = getStaticOutputData<Image>(0);
    std::cout << "GOT DATA" << std::endl;

    // Initialize output image
    ExecutionDevice::pointer device = getMainDevice();
    if(mOutputTypeSet) {
        output->create(input->getSize(), mOutputType, input->getNrOfComponents());
        output->setSpacing(input->getSpacing());
    } else {
        output->createFromImage(input);
    }
    mOutputType = output->getDataType();
    SceneGraph::setParentNode(output, input);
    
    
    if(device->isHost()) {
        switch(input->getDataType()) {
                fastSwitchTypeMacro(executeAlgorithmOnHost<FAST_TYPE>(input, output, groupSize, windowSize, denoiseStrength, sigma));
        }
    } else {
        OpenCLDevice::pointer clDevice = device;
        
        recompileOpenCLCode(input);
        
        cl::NDRange globalSize;
        
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        if(input->getDimensions() == 2) {
            std::cout << "DOING CL" << std::endl;
            OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            mKernel.setArg(2, (denoiseStrength*denoiseStrength));
            mKernel.setArg(3, (sigma*sigma));
            globalSize = cl::NDRange(input->getWidth(),input->getHeight());
            mKernel.setArg(0, *inputAccess->get2DImage());
            mKernel.setArg(1, *outputAccess->get2DImage());
            clDevice->getCommandQueue().enqueueNDRangeKernel(
                    mKernel,
                    cl::NullRange,
                    globalSize,
                    cl::NullRange
            );
        } else {
            // Create an auxilliary image
            //Image::pointer output2 = Image::New();
            //output2->createFromImage(output);
            
            globalSize = cl::NDRange(input->getWidth(),input->getHeight(),input->getDepth());
            
            if(clDevice->isWritingTo3DTexturesSupported()) {
                mKernel.setArg(2, (denoiseStrength*denoiseStrength));
                mKernel.setArg(3, (sigma*sigma));
                OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
                mKernel.setArg(0, *inputAccess->get3DImage());
                mKernel.setArg(1, *outputAccess->get3DImage());
                clDevice->getCommandQueue().enqueueNDRangeKernel(
                        mKernel,
                        cl::NullRange,
                        globalSize,
                        cl::NullRange
                );
            }else{
                mKernel.setArg(2, (denoiseStrength*denoiseStrength));
                mKernel.setArg(3, (sigma*sigma));
                OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
                mKernel.setArg(0, *inputAccess->get3DImage());
                mKernel.setArg(1, *outputAccess->get());
                clDevice->getCommandQueue().enqueueNDRangeKernel(
                        mKernel,
                        cl::NullRange,
                        globalSize,
                        cl::NullRange
                );
            }
            
            
        }
    }
}

/*
void NoneLocalMeans::execute() {
	Image::pointer input = getStaticInputData<Image>(0);
	Image::pointer output = getStaticOutputData<Image>(0);
    
	// Initialize output image
	ExecutionDevice::pointer device = getMainDevice();
    
	if (mOutputTypeSet) {
        output->create(input->getSize(), mOutputType, input->getNrOfComponents());
        output->setSpacing(input->getSpacing());
	}else {
		output->createFromImage(input);
	}
	mOutputType = output->getDataType();

	if (device->isHost()) {
		switch (input->getDataType()) {
			fastSwitchTypeMacro(executeAlgorithmOnHost<FAST_TYPE>(input, output, groupSize, windowSize, denoiseStrength, sigma));
		}
	}
	else {
		OpenCLDevice::pointer clDevice = device;

		recompileOpenCLCode(input);
		cl::NDRange globalSize;
		if (input->getDimensions() == 2) {
			globalSize = cl::NDRange(input->getWidth(), input->getHeight());

			OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
			OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            
            //Her trenger jeg og legge til sym image
            //Det skal ta input, og extends sidene likt
            //MinX/MinY = 0 - ((window-1)/2) - ((group-1)/2)
            //MaxX/MaxY = max + ((window-1)/2) + ((group-1)/2)
                        
			mKernel.setArg(0, *inputAccess->get2DImage());
			mKernel.setArg(1, *outputAccess->get2DImage());
		}
		else {
			globalSize = cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth());

			const bool writingTo3DTextures = clDevice->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos;
			OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
			mKernel.setArg(0, *inputAccess->get3DImage());
			if (writingTo3DTextures) {
				OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
				mKernel.setArg(1, *outputAccess->get3DImage());
			}
			else {
				OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
				mKernel.setArg(1, *outputAccess->get());
			}
		}

		//mKernel.setArg(2, groupSize);
		//mKernel.setArg(3, windowSize);
		mKernel.setArg(2, (denoiseStrength*denoiseStrength));
		mKernel.setArg(3, sigma);
        //mKernel.setArg(4, k);

		clDevice->getCommandQueue().enqueueNDRangeKernel(
			mKernel,
			cl::NullRange,
			globalSize,
			cl::NullRange
		);
	}
}*/
float NonLocalMeans::getSigma(){
    return sigma;
}

float NonLocalMeans::getDenoiseStrength(){
    return denoiseStrength;
}

int NonLocalMeans::getGroupSize(){
    return groupSize;
}

int NonLocalMeans::getWindowSize(){
    return windowSize;
}

int NonLocalMeans::getK(){
    return k;
}

void NonLocalMeans::waitToFinish() {
    if (!getMainDevice()->isHost()) {
        OpenCLDevice::pointer device = getMainDevice();
        device->getCommandQueue().finish();
    }
}
