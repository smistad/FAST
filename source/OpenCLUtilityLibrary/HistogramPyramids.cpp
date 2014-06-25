#include "HistogramPyramids.hpp"
#include <cmath>
#include <iostream>
#include "OulConfig.hpp"


using namespace cl;
using namespace oul;

// Undefine windows crap
#undef min
#undef max

double log2( double n )  
{  
    return log( n ) / log( 2.0 );  
}

double round( double n )
{
	return (n - floor(n) > 0.5) ? ceil(n) : floor(n);
}

void HistogramPyramid::compileCode(oul::Context &context) {
    // Check if context has the program, if not compile it
    if(!context.hasProgram("oul::HistogramPyramids")) {
        context.createProgramFromSourceWithName("oul::HistogramPyramids", std::string(OUL_DIR)+"/HistogramPyramids.cl", "-I " + std::string(OUL_DIR) + "");
    }
}

HistogramPyramid2D::HistogramPyramid2D(oul::Context &context) {
    compileCode(context);
    this->context = context;
}
HistogramPyramid3D::HistogramPyramid3D(oul::Context &context) {
    // TODO : check if device in context support writing to 3D images
    compileCode(context);
    this->context = context;
}
HistogramPyramid3DBuffer::HistogramPyramid3DBuffer(oul::Context &context) {
    compileCode(context);
    this->context = context;
}

int HistogramPyramid::getSum() {
    return this->sum;
}

void HistogramPyramid3D::create(Image3D &baseLevel, int sizeX, int sizeY, int sizeZ) {
    // Make baseLevel into power of 2 in all dimensions
    if(sizeX == sizeY && sizeY == sizeZ && log2(sizeX) == round(log2(sizeX))) {
        size = sizeX;
    }else{
        // Find largest size and find closest power of two
        int largestSize = std::max(sizeX, std::max(sizeY, sizeZ));
        int i = 1;
        while(pow(2.0, i) < largestSize)
            i++;
        size = pow(2.0, i);
    }
    std::cout << "3D HP size: " << size << std::endl;

    // Create all levels
    HPlevels.push_back(baseLevel);
    int levelSize = size / 2;
    HPlevels.push_back(Image3D(
                context.getContext(),
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT8),
                levelSize, levelSize, levelSize
    ));
    levelSize /= 2;
    HPlevels.push_back(Image3D(
                context.getContext(),
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT8),
                levelSize, levelSize, levelSize
    ));
    levelSize /= 2;
    // 16 bit
    HPlevels.push_back(Image3D(
                context.getContext(),
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT16),
                levelSize, levelSize, levelSize
    ));
    levelSize /= 2;
    HPlevels.push_back(Image3D(
                context.getContext(),
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT16),
                levelSize, levelSize, levelSize
    ));
    levelSize /= 2;

    // The rest will use 32 bit
    for(int i = 5; i < log2(size); i++) {
        HPlevels.push_back(Image3D(
                    context.getContext(),
                    CL_MEM_READ_WRITE,
                    ImageFormat(CL_R, CL_UNSIGNED_INT32),
                    levelSize, levelSize, levelSize
        ));
        levelSize /= 2;
    }

    // Do construction iterations
    cl::Program program = context.getProgram("oul::HistogramPyramids");
    cl::CommandQueue queue = context.getQueue(0);
    Kernel constructHPLevelKernel(program, "constructHPLevel3D");
    levelSize = size;
    for(int i = 0; i < log2((float)size)-1; i++) {
        constructHPLevelKernel.setArg(0, HPlevels[i]);
        constructHPLevelKernel.setArg(1, HPlevels[i+1]);
        levelSize /= 2;
        queue.enqueueNDRangeKernel(
            constructHPLevelKernel,
            NullRange,
            NDRange(levelSize, levelSize, levelSize),
            NullRange
        );
    }

    // Get total sum and return it
    int * sum = new int[8];
    cl::size_t<3> offset;
    offset[0] = 0;
    offset[1] = 0;
    offset[2] = 0;
    cl::size_t<3> region;
    region[0] = 2;
    region[1] = 2;
    region[2] = 2;
    queue.enqueueReadImage(HPlevels[HPlevels.size()-1], CL_TRUE, offset, region, 0, 0, sum);
    //for(int i = 0; i < 8; i++)
    //std::cout << sum[i] << std::endl;
    this->sum = sum[0] + sum[1] + sum[2] + sum[3] + sum[4] + sum[5] + sum[6] + sum[7];
}

void HistogramPyramid3DBuffer::create(Buffer &baseLevel, int sizeX, int sizeY, int sizeZ) {
    this->sizeX = sizeX;
    this->sizeY = sizeY;
    this->sizeZ = sizeZ;
    // Make baseLevel into power of 2 in all dimensions
    if(sizeX == sizeY && sizeY == sizeZ && log2(sizeX) == round(log2(sizeX))) {
        size = sizeX;
    }else{
        // Find largest size and find closest power of two
        int largestSize = std::max(sizeX, std::max(sizeY, sizeZ));
        int i = 1;
        while(pow(2.0, i) < largestSize)
            i++;
        size = pow(2.0, i);
    }
    std::cout << "3D HP size: " << size << std::endl;


    // Create all levels
    HPlevels.push_back(baseLevel);
    int levelSize = size*size*size / 8;
    HPlevels.push_back(Buffer(context.getContext(), CL_MEM_READ_WRITE, sizeof(char)*levelSize));
    levelSize /= 8;
    HPlevels.push_back(Buffer(context.getContext(), CL_MEM_READ_WRITE, sizeof(short)*levelSize));
    levelSize /= 8;
    HPlevels.push_back(Buffer(context.getContext(), CL_MEM_READ_WRITE, sizeof(short)*levelSize));
    levelSize /= 8;
    HPlevels.push_back(Buffer(context.getContext(), CL_MEM_READ_WRITE, sizeof(short)*levelSize));
    levelSize /= 8;
    for(int i = 5; i < (log2((float)size)); i ++) {
        HPlevels.push_back(Buffer(context.getContext(), CL_MEM_READ_WRITE, sizeof(int)*levelSize));
        levelSize /= 8;
    }
    cl::Program program = context.getProgram("oul::HistogramPyramids");
    Kernel constructHPLevelCharCharKernel = Kernel(program, "constructHPLevelCharChar");
    Kernel constructHPLevelCharShortKernel = Kernel(program, "constructHPLevelCharShort");
    Kernel constructHPLevelShortShortKernel = Kernel(program, "constructHPLevelShortShort");
    Kernel constructHPLevelShortIntKernel = Kernel(program, "constructHPLevelShortInt");
    Kernel constructHPLevelKernel = Kernel(program, "constructHPLevelBuffer");

    // Run base to first level
    constructHPLevelCharCharKernel.setArg(0, HPlevels[0]);
    constructHPLevelCharCharKernel.setArg(1, HPlevels[1]);
    constructHPLevelCharCharKernel.setArg(2, sizeX);
    constructHPLevelCharCharKernel.setArg(3, sizeY);
    constructHPLevelCharCharKernel.setArg(4, sizeZ);

    cl::CommandQueue queue = context.getQueue(0);
    queue.enqueueNDRangeKernel(
        constructHPLevelCharCharKernel,
        NullRange,
        NDRange(size/2, size/2, size/2),
        NullRange
    );

    int previous = size / 2;

    constructHPLevelCharShortKernel.setArg(0, HPlevels[1]);
    constructHPLevelCharShortKernel.setArg(1, HPlevels[2]);

    queue.enqueueNDRangeKernel(
        constructHPLevelCharShortKernel,
        NullRange,
        NDRange(previous/2, previous/2, previous/2),
        NullRange
    );

    previous /= 2;

    constructHPLevelShortShortKernel.setArg(0, HPlevels[2]);
    constructHPLevelShortShortKernel.setArg(1, HPlevels[3]);

    queue.enqueueNDRangeKernel(
        constructHPLevelShortShortKernel,
        NullRange,
        NDRange(previous/2, previous/2, previous/2),
        NullRange
    );

    previous /= 2;

    constructHPLevelShortShortKernel.setArg(0, HPlevels[3]);
    constructHPLevelShortShortKernel.setArg(1, HPlevels[4]);

    queue.enqueueNDRangeKernel(
        constructHPLevelShortShortKernel,
        NullRange,
        NDRange(previous/2, previous/2, previous/2),
        NullRange
    );

    previous /= 2;

    constructHPLevelShortIntKernel.setArg(0, HPlevels[4]);
    constructHPLevelShortIntKernel.setArg(1, HPlevels[5]);

    queue.enqueueNDRangeKernel(
        constructHPLevelShortIntKernel,
        NullRange,
        NDRange(previous/2, previous/2, previous/2),
        NullRange
    );

    previous /= 2;

    // Run level 2 to top level
    for(int i = 5; i < log2((float)size)-1; i++) {
        constructHPLevelKernel.setArg(0, HPlevels[i]);
        constructHPLevelKernel.setArg(1, HPlevels[i+1]);
        previous /= 2;
        queue.enqueueNDRangeKernel(
            constructHPLevelKernel,
            NullRange,
            NDRange(previous, previous, previous),
            NullRange
        );
    }

    int * sum = new int[8];
    queue.enqueueReadBuffer(HPlevels[HPlevels.size()-1], CL_TRUE, 0, sizeof(int)*8, sum);
    this->sum = sum[0] + sum[1] + sum[2] + sum[3] + sum[4] + sum[5] + sum[6] + sum[7];
}

void HistogramPyramid2D::create(Image2D &baseLevel, int sizeX, int sizeY) {
    // Make baseLevel into power of 2 in all dimensions
    if(sizeX == sizeY && log2(sizeX) == round(log2(sizeX))) {
        size = sizeX;
    } else {
        // Find largest size and find closest power of two
        int largestSize = std::max(sizeX, sizeY);
        int i = 1;
        while(pow(2.0, i) < largestSize)
            i++;
        size = pow(2.0, i);
    }
    std::cout << "2D HP size: " << size << std::endl;

    // Create all levels
    HPlevels.push_back(baseLevel);
    int levelSize = size / 2;
    HPlevels.push_back(Image2D(
                context.getContext(),
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT8),
                levelSize, levelSize
    ));
    levelSize /= 2;
    HPlevels.push_back(Image2D(
                context.getContext(),
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT8),
                levelSize, levelSize
    ));
    levelSize /= 2;
    // 16 bit
    HPlevels.push_back(Image2D(
                context.getContext(),
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT16),
                levelSize, levelSize
    ));
    levelSize /= 2;
    HPlevels.push_back(Image2D(
                context.getContext(),
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT16),
                levelSize, levelSize
    ));
    levelSize /= 2;

    // The rest will use 32 bit
    for(int i = 5; i < log2(size); i++) {
        HPlevels.push_back(Image2D(
                    context.getContext(),
                    CL_MEM_READ_WRITE,
                    ImageFormat(CL_R, CL_UNSIGNED_INT32),
                    levelSize, levelSize
        ));
        levelSize /= 2;
    }

    // Do construction iterations
    cl::Program program = context.getProgram("oul::HistogramPyramids");
    cl::CommandQueue queue = context.getQueue(0);
    Kernel constructHPLevelKernel(program, "constructHPLevel2D");
    levelSize = size;
    for(int i = 0; i < log2((float)size)-1; i++) {
        constructHPLevelKernel.setArg(0, HPlevels[i]);
        constructHPLevelKernel.setArg(1, HPlevels[i+1]);
        levelSize /= 2;
        queue.enqueueNDRangeKernel(
            constructHPLevelKernel,
            NullRange,
            NDRange(levelSize, levelSize),
            NullRange
        );
    }

    // Get total sum and return it
    unsigned int * sum = new unsigned int[4];
    cl::size_t<3> offset;
    offset[0] = 0;
    offset[1] = 0;
    offset[2] = 0;
    cl::size_t<3> region;
    region[0] = 2;
    region[1] = 2;
    region[2] = 1;
    queue.enqueueReadImage(HPlevels[HPlevels.size()-1], CL_TRUE, offset, region, 0, 0, sum);
    this->sum = sum[0] + sum[1] + sum[2] + sum[3];
}

void HistogramPyramid2D::traverse(Kernel &kernel, int arguments) {
    for(int i = 0; i < 14; i++) {
        int l = i;
        if(i >= HPlevels.size())
            // if not using all levels, just add the last levels as dummy arguments
            l = HPlevels.size()-1;
        kernel.setArg(i+arguments, HPlevels[l]);
    }

    int global_work_size = sum + 64 - (sum - 64*(sum / 64));
    context.getQueue(0).enqueueNDRangeKernel(kernel, NullRange, NDRange(global_work_size), NDRange(64));
}

void HistogramPyramid3D::traverse(Kernel &kernel, int arguments) {
    kernel.setArg(arguments, this->size);
    kernel.setArg(arguments+1, this->sum);
    for(int i = 0; i < 10; i++) {
        int l = i;
        if(i >= HPlevels.size())
            // if not using all levels, just add the last levels as dummy arguments
            l = HPlevels.size()-1;
        kernel.setArg(i+arguments+2, HPlevels[l]);
    }

    int global_work_size = sum + 64 - (sum - 64*(sum / 64));
    context.getQueue(0).enqueueNDRangeKernel(kernel, NullRange, NDRange(global_work_size), NDRange(64));
}

void HistogramPyramid3DBuffer::traverse(Kernel &kernel, int arguments) {
    kernel.setArg(arguments, this->size);
    kernel.setArg(arguments+1, this->sum);
    for(int i = 0; i < 10; i++) {
        int l = i;
        if(i >= HPlevels.size())
            // if not using all levels, just add the last levels as dummy arguments
            l = HPlevels.size()-1;
        kernel.setArg(i+arguments+2, HPlevels[l]);
    }

    int global_work_size = sum + 64 - (sum - 64*(sum / 64));
    context.getQueue(0).enqueueNDRangeKernel(kernel, NullRange, NDRange(global_work_size), NDRange(64));
}


Buffer HistogramPyramid2D::createPositionBuffer() {
    Buffer * positions = new Buffer(
            context.getContext(),
            CL_MEM_READ_WRITE,
            2*sizeof(int)*sum
    );
    Kernel kernel(context.getProgram("oul::HistogramPyramids"), "createPositions2D");
    kernel.setArg(0, (*positions));
    kernel.setArg(1, this->size);
    kernel.setArg(2, this->sum);
    this->traverse(kernel, 3);
    return *positions;
}

Buffer HistogramPyramid3D::createPositionBuffer() {
    Buffer * positions = new Buffer(
            context.getContext(),
            CL_MEM_READ_WRITE,
            3*sizeof(int)*sum
    );
    Kernel kernel(context.getProgram("oul::HistogramPyramids"), "createPositions3D");
    kernel.setArg(0, (*positions));
    this->traverse(kernel, 1);
    return *positions;
}

Buffer HistogramPyramid3DBuffer::createPositionBuffer() {
    Buffer * positions = new Buffer(
            context.getContext(),
            CL_MEM_READ_WRITE,
            3*sizeof(int)*sum
    );
    Kernel kernel(context.getProgram("oul::HistogramPyramids"), "createPositions3DBuffer");
    kernel.setArg(0, sizeX);
    kernel.setArg(1, sizeY);
    kernel.setArg(2, sizeZ);
    kernel.setArg(3, (*positions));
    this->traverse(kernel, 4);
    return *positions;
}

void HistogramPyramid2D::deleteHPlevels() {
    HPlevels.clear();
}

void HistogramPyramid3D::deleteHPlevels() {
    HPlevels.clear();
}

void HistogramPyramid3DBuffer::deleteHPlevels() {
    HPlevels.clear();
}
