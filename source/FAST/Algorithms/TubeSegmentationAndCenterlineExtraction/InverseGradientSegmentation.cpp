#include "InverseGradientSegmentation.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Utility.hpp"

namespace fast {

void InverseGradientSegmentation::setCenterlineInputConnection(
        DataPort::pointer port) {
    setInputConnection(0, port);
}

void InverseGradientSegmentation::setVectorFieldInputConnection(
        DataPort::pointer port) {
    setInputConnection(1, port);
}

InverseGradientSegmentation::InverseGradientSegmentation() {
    createInputPort<Segmentation>(0);
    createInputPort<Image>(1);
    createOutputPort<Segmentation>(0);
}

void InverseGradientSegmentation::execute() {
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    bool no3Dwrite = !device->isWritingTo3DTexturesSupported();
    Segmentation::pointer centerline = getInputData<Segmentation>(0);
    Vector3ui size = centerline->getSize();
    Image::pointer vectorField = getInputData<Image>(1);
    Segmentation::pointer segmentation = getOutputData<Segmentation>(0);
    segmentation->createFromImage(centerline);
    SceneGraph::setParentNode(segmentation, centerline);
    Segmentation::pointer segmentation2 = Segmentation::New();
    segmentation2->createFromImage(centerline);

    OpenCLImageAccess::pointer centerlineAccess = centerline->getOpenCLImageAccess(ACCESS_READ, device);
    OpenCLImageAccess::pointer vectorFieldAccess = vectorField->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    OpenCLImageAccess::pointer segmentationOutputAccess = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    cl::Image3D* volume = segmentationOutputAccess->get3DImage();

    device->createProgramFromSourceWithName("inverseGradientSegmentation",
            Config::getKernelSourcePath() + "Algorithms/TubeSegmentationAndCenterlineExtraction/InverseGradientSegmentation.cl");
    cl::Program program(device->getProgram("inverseGradientSegmentation"));


    cl::Kernel initGrowKernel(program, "initGrowing");
    cl::Kernel growKernel(program, "grow");

    cl::CommandQueue queue = device->getCommandQueue();


    cl::size_t<3> offset = createOrigoRegion();
    cl::size_t<3> region = createRegion(size.x(), size.y(), size.z());

    // Copy centerline to segmentation
    queue.enqueueCopyImage(
            *(centerlineAccess->get3DImage()),
            *volume,
            offset,
            offset,
            region
    );

    int stopGrowing = 0;
    cl::Buffer stop = cl::Buffer(device->getContext(), CL_MEM_WRITE_ONLY, sizeof(int));
    queue.enqueueWriteBuffer(stop, CL_FALSE, 0, sizeof(int), &stopGrowing);

    growKernel.setArg(1, *(vectorFieldAccess->get3DImage()));
    growKernel.setArg(3, stop);

    int i = 0;
    int minimumIterations = 0;
    if(!device->isWritingTo3DTexturesSupported()) {
        OpenCLBufferAccess::pointer segmentation2Access = segmentation2->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        cl::Buffer* volume2 = segmentation2Access->get();
        queue.enqueueCopyImageToBuffer(
                *volume,
                *volume2,
                offset,
                region,
                0
        );
        initGrowKernel.setArg(0, *volume);
        initGrowKernel.setArg(1, *volume2);
        //initGrowKernel.setArg(2, radius);
        queue.enqueueNDRangeKernel(
            initGrowKernel,
            cl::NullRange,
            cl::NDRange(size.x(), size.y(), size.z()),
            cl::NullRange
        );
        queue.enqueueCopyBufferToImage(
                *volume2,
                *volume,
                0,
                offset,
                region
        );
        growKernel.setArg(0, *volume);
        growKernel.setArg(2, *volume2);
        while(stopGrowing == 0) {
            if(i > minimumIterations) {
                stopGrowing = 1;
                queue.enqueueWriteBuffer(stop, CL_TRUE, 0, sizeof(int), &stopGrowing);
            }

            queue.enqueueNDRangeKernel(
                    growKernel,
                    cl::NullRange,
                    cl::NDRange(size.x(), size.y(), size.z()),
                    cl::NullRange
            );
            if(i > minimumIterations)
                queue.enqueueReadBuffer(stop, CL_TRUE, 0, sizeof(int), &stopGrowing);
            i++;
            queue.enqueueCopyBufferToImage(
                    *volume2,
                    *volume,
                    0,
                    offset,
                    region
            );
        }
    } else {
        OpenCLImageAccess::pointer segmentation2Access = segmentation2->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        cl::Image3D* volume2 = segmentation2Access->get3DImage();
        queue.enqueueCopyImage(
                *volume,
                *volume2,
                createOrigoRegion(),
                createOrigoRegion(),
                createRegion(size.x(), size.y(), size.z())
        );
        initGrowKernel.setArg(0, *volume);
        initGrowKernel.setArg(1, *volume2);
        //initGrowKernel.setArg(2, radius);
        queue.enqueueNDRangeKernel(
            initGrowKernel,
            cl::NullRange,
            cl::NDRange(size.x(), size.y(), size.z()),
            cl::NullRange
        );
        while(stopGrowing == 0) {
            if(i > minimumIterations) {
                stopGrowing = 1;
                queue.enqueueWriteBuffer(stop, CL_FALSE, 0, sizeof(int), &stopGrowing);
            }
            if(i % 2 == 0) {
                growKernel.setArg(0, *volume);
                growKernel.setArg(2, *volume2);
            } else {
                growKernel.setArg(0, *volume2);
                growKernel.setArg(2, *volume);
            }

            queue.enqueueNDRangeKernel(
                    growKernel,
                    cl::NullRange,
                    cl::NDRange(size.x(), size.y(), size.z()),
                    cl::NullRange
            );
            if(i > minimumIterations)
                queue.enqueueReadBuffer(stop, CL_TRUE, 0, sizeof(int), &stopGrowing);
            i++;
        }

    }

    std::cout << "segmentation result grown in " << i << " iterations" << std::endl;

    cl::Kernel dilateKernel(program, "dilate");
    cl::Kernel erodeKernel(program, "erode");

    if(!device->isWritingTo3DTexturesSupported()) {
        OpenCLBufferAccess::pointer segmentation2Access = segmentation2->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        cl::Buffer* volumeBuffer = segmentation2Access->get();
        dilateKernel.setArg(0, *volume);
        dilateKernel.setArg(1, *volumeBuffer);

        queue.enqueueNDRangeKernel(
            dilateKernel,
            cl::NullRange,
            cl::NDRange(size.x(), size.y(), size.z()),
            cl::NullRange
        );

        queue.enqueueCopyBufferToImage(
                *volumeBuffer,
                *volume,
                0,
                offset,
                region);

        erodeKernel.setArg(0, *volume);
        erodeKernel.setArg(1, *volumeBuffer);

        queue.enqueueNDRangeKernel(
            erodeKernel,
            cl::NullRange,
            cl::NDRange(size.x(), size.y(), size.z()),
            cl::NullRange
        );
        queue.enqueueCopyBufferToImage(
            *volumeBuffer,
            *volume,
            0,
            offset,
            region
        );
    } else {
        OpenCLImageAccess::pointer segmentation2Access = segmentation2->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        cl::Image3D* volume2 = segmentation2Access->get3DImage();

        dilateKernel.setArg(0, *volume);
        dilateKernel.setArg(1, *volume2);

        queue.enqueueNDRangeKernel(
            dilateKernel,
            cl::NullRange,
            cl::NDRange(size.x(), size.y(), size.z()),
            cl::NullRange
        );

        erodeKernel.setArg(0, *volume2);
        erodeKernel.setArg(1, *volume);

        queue.enqueueNDRangeKernel(
            erodeKernel,
            cl::NullRange,
            cl::NDRange(size.x(), size.y(), size.z()),
            cl::NullRange
        );
    }

}


}
