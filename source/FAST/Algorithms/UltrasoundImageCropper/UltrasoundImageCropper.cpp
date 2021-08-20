#include "UltrasoundImageCropper.hpp"
#include "FAST/Data/Image.hpp"

#include "FAST/Utility.hpp"

namespace fast {

void UltrasoundImageCropper::loadAttributes() {

}

UltrasoundImageCropper::UltrasoundImageCropper(float physicalWidth, bool staticCropping, float threshold1, float threshold2) {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.cl");
    if(physicalWidth > 0)
        setPhysicalWidth(physicalWidth);
    setStaticCropping(staticCropping);
    setThresholds(threshold1, threshold2);
}

void UltrasoundImageCropper::setThresholds(float threshold1, float threshold2) {
    m_threshold1 = threshold1;
    m_threshold2 = threshold2;
}

void UltrasoundImageCropper::execute() {
    Image::pointer image = getInputData<Image>();

    if(image->getDimensions() != 2) {
        throw Exception("THe UltrasoundImageCropper is only for 2D images");
    }

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();
    OpenCLImageAccess::pointer imageAccess = image->getOpenCLImageAccess(ACCESS_READ, device);

    if(m_width == -1 || !m_staticCropping) {
        cl::Program program = getOpenCLProgram(device);
        cl::Kernel kernel(program, "lineSearch");


        const int width = image->getWidth();
        const int height = image->getHeight();

        cl::Buffer rays(
                device->getContext(),
                CL_MEM_READ_WRITE,
                sizeof(uint) * (width + height)
        );

        kernel.setArg(0, *imageAccess->get2DImage());
        kernel.setArg(1, rays);

        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(width + height),
                cl::NullRange
        );

        // Results contains the amount of non-zero values per column and row
        auto result = make_uninitialized_unique<uint[]>(width + height);
        queue.enqueueReadBuffer(rays, CL_TRUE, 0, sizeof(uint) * (width + height), result.get());
        int minX = 0;
        int maxX = width;
        int minY = 0;
        int maxY = height;
        for(int x = width / 2; x > 0; --x) {
            if(result[x] <= m_threshold1) {
                minX = x;
                break;
            }
        }
        for(int x = width / 2; x < width; ++x) {
            if(result[x] <= m_threshold1) {
                maxX = x;
                break;
            }
        }
        for(int y = height / 4; y > 0; --y) {
            if(result[width + y] <= m_threshold2) {
                minY = y;
                break;
            }
        }
        for(int y = height / 2 + height / 4; y < height; ++y) {
            if(result[width + y] <= m_threshold2) {
                maxY = y;
                break;
            }
        }

        // Crop image
        m_offsetX = minX;
        m_offsetY = minY;
        m_width = maxX - minX;
        m_height = maxY - minY;
        if(m_width <= 0 || m_height <= 0)
            throw Exception("Error occured in UltrasoundImageCropper. Incorrect size.");
        if(m_physicalWidth > 0) {
            // Calculate physical height of image
            const float spacing1 = m_physicalWidth / m_width;

            m_spacing = Vector3f(spacing1, spacing1, 1.0f);
        } else {
            m_spacing = image->getSpacing();
        }
    }

    auto outputImage = Image::create(m_width, m_height, image->getDataType(), image->getNrOfChannels());
    outputImage->setSpacing(m_spacing);
    outputImage->setCreationTimestamp(image->getCreationTimestamp());
	outputImage->setFrameData("original-width", std::to_string(outputImage->getWidth()));
	outputImage->setFrameData("original-height", std::to_string(outputImage->getHeight()));

    OpenCLImageAccess::pointer outputAccess = outputImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    queue.enqueueCopyImage(
            *imageAccess->get2DImage(),
            *outputAccess->get2DImage(),
            createRegion(m_offsetX, m_offsetY, 0),
            createOrigoRegion(),
            createRegion(m_width, m_height, 1)
    );

    addOutputData(0, outputImage);
}

void UltrasoundImageCropper::setPhysicalWidth(float width) {
    if(width <= 0)
        throw Exception("Physical width must be > 0");
    m_physicalWidth = width;
}

void UltrasoundImageCropper::setStaticCropping(bool staticCropping) {
    m_staticCropping = staticCropping;
}


} // end namespace fast
