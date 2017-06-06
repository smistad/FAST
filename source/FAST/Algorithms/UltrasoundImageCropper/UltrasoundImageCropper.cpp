#include "UltrasoundImageCropper.hpp"
#include "FAST/Data/Image.hpp"

#include "FAST/Utility.hpp"

namespace fast {


UltrasoundImageCropper::UltrasoundImageCropper() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);

    createOpenCLProgram(Config::getKernelSourcePath() + "/UltrasoundImageCropper/UltrasoundImageCropper.cl");
}

void UltrasoundImageCropper::execute() {
    reportInfo() << "EXECUTING THE IMAGE CROPPER" << Reporter::end();
    Image::pointer image = getStaticInputData<Image>();

    if(image->getDimensions() != 2) {
        throw Exception("THe UltrasoundImageCropper is only for 2D images");
    }


    OpenCLDevice::pointer device = getMainDevice();
    cl::Program program = getOpenCLProgram(device);
    cl::Kernel kernel(program, "lineSearch");

    OpenCLImageAccess::pointer imageAccess = image->getOpenCLImageAccess(ACCESS_READ, device);

    const uint width = image->getWidth();
    const uint height = image->getHeight();

    cl::Buffer rays(
            device->getContext(),
            CL_MEM_READ_WRITE,
            sizeof(uchar)*(width + height)
    );


    kernel.setArg(0, *imageAccess->get2DImage());
    kernel.setArg(1, rays);

    cl::CommandQueue queue = device->getCommandQueue();

    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(width + height),
            cl::NullRange
    );

    UniquePointer<uchar[]> result(new uchar[width + height]);
    queue.enqueueReadBuffer(rays, CL_TRUE, 0, sizeof(uchar)*(width + height), result.get());
    int minX = 0;
    int maxX = width;
    int minY = 0;
    int maxY = height;
    for(int x = 0; x < width; ++x) {
        if(result[x] > 0) {
            minX = x;
            break;
        }
    }
    for(int x = width - 1; x > 0; --x) {
        if(result[x] > 0) {
            maxX = x;
            break;
        }
    }
    for(int y = 0; y < height; ++y) {
        if(result[width + y] > 0) {
            minY = y;
            break;
        }
    }
    for(int y = height - 1; y > 0; --y) {
        if(result[width + y] > 0) {
            maxY = y;
            break;
        }
    }

    // Crop image
    const int newWidth = maxX - minX;
    const int newHeight = maxY - minY;
    reportInfo() << "Min/Max X and Y " << minX << " " << maxX << " " << minY << " " << maxY << Reporter::end();
    reportInfo() << "Cropped image to size " << newWidth << " " << newHeight << Reporter::end();
    Image::pointer outputImage = getStaticOutputData<Image>();
    outputImage->create(newWidth, newHeight, image->getDataType(), image->getNrOfComponents());
    outputImage->setSpacing(image->getSpacing());
    outputImage->setCreationTimestamp(image->getCreationTimestamp());

    OpenCLImageAccess::pointer outputAccess = outputImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    queue.enqueueCopyImage(
            *imageAccess->get2DImage(),
            *outputAccess->get2DImage(),
            createRegion(minX, minY, 0),
            createOrigoRegion(),
            createRegion(newWidth, newHeight, 1)
    );

}


} // end namespace fast
