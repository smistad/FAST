#include "FAST/Algorithms/Skeletonization/Skeletonization.hpp"
#include "FAST/DeviceManager.hpp"
#include "HelperFunctions.hpp"
#include "FAST/SceneGraph.hpp"
#include "FAST/Data/Segmentation.hpp"

namespace fast {

Skeletonization::Skeletonization() {
    createInputPort<Segmentation>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
}

void Skeletonization::execute() {
    Segmentation::pointer input = getStaticInputData<Segmentation>();
    Image::pointer output = getStaticOutputData<Image>();
    SceneGraph::setParentNode(output, input);

    if(input->getDimensions() != 2)
        throw Exception("The skeletonization algorithm currently only support 2D images");

    // Initialize output image
    output->createFromImage(input, getMainDevice());

    OpenCLDevice::pointer device = getMainDevice();

    // Create kernel
    int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "Algorithms/Skeletonization/Skeletonization2D.cl");
    cl::Program program = device->getProgram(programNr);
    cl::Kernel kernel1(program, "thinningStep1");
    cl::Kernel kernel2(program, "thinningStep2");

    // Create buffer for check if stop
    cl::Buffer stopGrowingBuffer = cl::Buffer(
            device->getContext(),
            CL_MEM_READ_WRITE,
            sizeof(char));

    cl::Image2D image2(device->getContext(), CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_UNSIGNED_INT8), output->getWidth(), output->getHeight());
    OpenCLImageAccess2D::pointer access = input->getOpenCLImageAccess2D(ACCESS_READ, device);
    cl::Image2D* image = access->get();
    OpenCLImageAccess2D::pointer access2 = output->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
    cl::Image2D* image1 = access2->get();

    kernel1.setArg(0, *image1);
    kernel1.setArg(1, image2);
    kernel1.setArg(2, stopGrowingBuffer);

    kernel2.setArg(0, image2);
    kernel2.setArg(1, *image1);
    kernel2.setArg(2, stopGrowingBuffer);

    cl::NDRange globalSize(output->getWidth(), output->getHeight());

    bool stopGrowing = false;
    char stopGrowingInit = 1;
    char* stopGrowingResult = new char;
    int iterations = 0;
    cl::CommandQueue queue = device->getCommandQueue();

    queue.enqueueCopyImage(
            *image,
            *image1,
            oul::createOrigoRegion(),
            oul::createOrigoRegion(),
            oul::createRegion(output->getWidth(), output->getHeight(), 1)
    );

    do {
        iterations++;
        queue.enqueueWriteBuffer(stopGrowingBuffer, CL_FALSE, 0, sizeof(char), &stopGrowingInit);

        queue.enqueueNDRangeKernel(
                kernel1,
                cl::NullRange,
                globalSize,
                cl::NullRange
        );

        queue.enqueueNDRangeKernel(
                kernel2,
                cl::NullRange,
                globalSize,
                cl::NullRange
        );

        queue.enqueueReadBuffer(stopGrowingBuffer, CL_TRUE, 0, sizeof(char), stopGrowingResult);
        if(*stopGrowingResult == 1)
            stopGrowing = true;
    } while(!stopGrowing);
    std::cout << "SKELETONIZATION EXECUTED" << std::endl;
}

} // end namespace fast
