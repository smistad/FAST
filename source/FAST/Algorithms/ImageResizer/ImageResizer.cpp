#include "ImageResizer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

void ImageResizer::setInterpolation(bool useInterpolation) {
    mInterpolationSet = true;
    mInterpolation = useInterpolation;
}

void ImageResizer::setWidth(int width) {
	if(width <= 0)
		throw Exception("Width must be larger than 0.");
	mSize.x() = width;
}

void ImageResizer::setHeight(int height) {
	if(height <= 0)
		throw Exception("Height must be larger than 0.");
	mSize.y() = height;
}

void ImageResizer::setDepth(int depth) {
	if(depth <= 0)
		throw Exception("Depth must be larger than 0.");
	mSize.z() = depth;
}

void ImageResizer::setSize(VectorXi size) {
	setWidth(size.x());
	setHeight(size.y());
	if(size.size() > 2)
		setDepth(size.z());
}

void ImageResizer::setPreserveAspectRatio(bool preserve) {
    mPreserveAspectRatio = preserve;
}

ImageResizer::ImageResizer() {
	createInputPort<Image>(0);
	createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageResizer/ImageResizer.cl");

	mSize = Vector3i::Zero();
    mPreserveAspectRatio = false;
    mInterpolationSet = false;
    mInterpolation = true;
}

void ImageResizer::execute() {
    Image::pointer input = getInputData<Image>();
    Image::pointer output = getOutputData<Image>();

    if(mSize.x() <= 0 || mSize.y() <= 0)
    	throw Exception("Desired size must be provided to ImageResizer");

    // Initialize output image
    if(input->getDimensions() == 2) {
        output->create(
                mSize.x(),
				mSize.y(),
                input->getDataType(),
                input->getNrOfChannels()
        );
    } else {
        if(mSize.z() == 0)
            throw Exception("Desired size must be provided to ImageResizer");
        output->create(
                mSize.cast<uint>(),
                input->getDataType(),
				input->getNrOfChannels()
        );
    }

    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {

        uchar useInterpolation = 1;
        if(mInterpolationSet) {
            useInterpolation = mInterpolation ? 1 : 0;
        }

        OpenCLDevice::pointer device = std::static_pointer_cast<OpenCLDevice>(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "");
        cl::Kernel kernel;
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        if(input->getDimensions() == 2) {
            if(mPreserveAspectRatio) {
                float scale = (float)input->getWidth() / output->getWidth();
                output->setSpacing(
                        input->getSpacing().x()*scale,
                        input->getSpacing().y()*scale,
                        1
                );
                int newHeight = (int)round(input->getHeight()/scale);
                kernel = cl::Kernel(program, "resize2DpreserveAspect");
                kernel.setArg(2, newHeight);
                kernel.setArg(3, useInterpolation);
            } else {
                output->setSpacing(Vector3f(
                    input->getSpacing().x()*((float)input->getWidth()/output->getWidth()),
                    input->getSpacing().y()*((float)input->getHeight()/output->getHeight()),
                    1.0f
                ));
                kernel = cl::Kernel(program, "resize2D");
                kernel.setArg(2, useInterpolation);
            }
            OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            kernel.setArg(0, *inputAccess->get2DImage());
            kernel.setArg(1, *outputAccess->get2DImage());
        } else {
            if(mPreserveAspectRatio)
                throw NotImplementedException();

            output->setSpacing(Vector3f(
                input->getSpacing().x()*((float)input->getWidth()/output->getWidth()),
                input->getSpacing().y()*((float)input->getHeight()/output->getHeight()),
                input->getSpacing().z()*((float)input->getDepth()/output->getDepth())
            ));
            kernel = cl::Kernel(program, "resize3D");
            kernel.setArg(0, *inputAccess->get3DImage());
            kernel.setArg(2, useInterpolation);

            if(device->isWritingTo3DTexturesSupported()) {
                OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
                kernel.setArg(1, *outputAccess->get3DImage());
            } else {
                if(input->getNrOfChannels() != 1)
                    throw Exception("ImageResizer does not support resizing for 3D images with more than 1 channel");
                // If device does not support writing to 3D textures, use a buffer instead
                OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
                kernel.setArg(1, *outputAccess->get());
            }
        }

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(output->getWidth(), output->getHeight(), output->getDepth()),
                cl::NullRange
        );
    }
}

}
