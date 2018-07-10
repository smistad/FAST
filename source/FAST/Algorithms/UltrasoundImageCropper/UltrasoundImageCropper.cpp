#include "UltrasoundImageCropper.hpp"
#include "FAST/Data/Image.hpp"

#include "FAST/Utility.hpp"

namespace fast {


UltrasoundImageCropper::UltrasoundImageCropper() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.cl");
}

void UltrasoundImageCropper::execute() {
    Image::pointer image = getInputData<Image>();

    if(image->getDimensions() != 2) {
        throw Exception("THe UltrasoundImageCropper is only for 2D images");
    }


    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::Program program = getOpenCLProgram(device);
    cl::Kernel kernel(program, "lineSearch");

    OpenCLImageAccess::pointer imageAccess = image->getOpenCLImageAccess(ACCESS_READ, device);

    const int width = image->getWidth();
    const int height = image->getHeight();

    cl::Buffer rays(
            device->getContext(),
            CL_MEM_READ_WRITE,
            sizeof(uint)*(width + height)
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

    // Results contains the amount of non-zero values per column and row
    auto result = make_uninitialized_unique<uint[]>(width + height);
    queue.enqueueReadBuffer(rays, CL_TRUE, 0, sizeof(uint)*(width + height), result.get());
    int minX = 0;
    int maxX = width;
    int minY = 0;
    int maxY = height;
    int threshold = 30;
    for(int x = width/2; x > 0; --x) {
        if(result[x] <= threshold) {
            minX = x;
            break;
        }
    }
    for(int x = width/2; x < width; ++x) {
        if(result[x] <= threshold) {
            maxX = x;
            break;
        }
    }
    int threshold2 = 10;
    for(int y = height/4; y > 0; --y) {
        if(result[width + y] <= threshold2) {
            minY = y;
            break;
        }
    }
    for(int y = height/2 + height/4; y < height; ++y) {
        if(result[width + y] <= threshold2) {
            maxY = y;
            break;
        }
    }

    // Crop image
    const int newWidth = maxX - minX;
    const int newHeight = maxY - minY;
    reportInfo() << "Min/Max X and Y " << minX << " " << maxX << " " << minY << " " << maxY << Reporter::end();
    reportInfo() << "Cropped image to size " << newWidth << " " << newHeight << Reporter::end();
    if(newWidth <= 0 || newHeight <= 0)
        throw Exception("Error occured in UltrasoundImageCropper. Incorrect size.");
    Image::pointer outputImage = getOutputData<Image>();
    outputImage->create(newWidth, newHeight, image->getDataType(), image->getNrOfChannels());
    if(m_physicalWidth > 0) {
        // Calculate physical height of image
        const float spacing1 = m_physicalWidth / newWidth;

        Vector3f spacing(spacing1, spacing1, 1.0f);
        outputImage->setSpacing(spacing);
    } else {
        outputImage->setSpacing(image->getSpacing());
    }
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

void UltrasoundImageCropper::setPhysicalWidth(float width) {
    if(width <= 0)
        throw Exception("Physical width must be > 0");
    m_physicalWidth = width;
}


} // end namespace fast
