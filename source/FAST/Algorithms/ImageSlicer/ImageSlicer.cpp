#include "ImageSlicer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

void ImageSlicer::setOrthogonalSlicePlane(PlaneType orthogonalSlicePlane,
		int sliceNr) {
	mOrthogonalSliceNr = sliceNr;
	mOrthogonalSlicePlane = orthogonalSlicePlane;
	mArbitrarySlicing = false;
	mOrthogonalSlicing = true;
}

void ImageSlicer::setArbitrarySlicePlane(Plane slicePlane) {
	mArbitrarySlicePlane = slicePlane;
	mArbitrarySlicing = true;
	mOrthogonalSlicing = false;
}

ImageSlicer::ImageSlicer() : mArbitrarySlicePlane(Plane(Vector3f(1,0,0))) {
	createInputPort<Image>(0);
	createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageSlicer/ImageSlicer.cl");

	mArbitrarySlicing = false;
	mOrthogonalSlicing = false;
}

void ImageSlicer::execute() {
	Image::pointer input = getStaticInputData<Image>();
	Image::pointer output = getStaticOutputData<Image>();

	if(input->getDimensions() != 3)
		throw Exception("Image slicer can only be used for 3D images");

	if(!mArbitrarySlicing && !mOrthogonalSlicing)
		throw Exception("No slice plane given to the ImageSlicer");

	// TODO
	if(mOrthogonalSlicing) {
		orthogonalSlicing(input, output);
	} else {
		arbitrarySlicing(input, output);
	}
}

void ImageSlicer::orthogonalSlicing(Image::pointer input, Image::pointer output) {
    OpenCLDevice::pointer device = getMainDevice();

    // Determine slice nr and width and height
    unsigned int sliceNr;
    if(mOrthogonalSliceNr < 0) {
        switch(mOrthogonalSlicePlane) {
        case PLANE_X:
            sliceNr = input->getWidth()/2;
            break;
        case PLANE_Y:
            sliceNr = input->getHeight()/2;
            break;
        case PLANE_Z:
            sliceNr = input->getDepth()/2;
            break;
        }
    } else {
        // Check that mSliceNr is valid
        sliceNr = mOrthogonalSliceNr;
        switch(mOrthogonalSlicePlane) {
        case PLANE_X:
            if(sliceNr >= input->getWidth())
                sliceNr = input->getWidth()-1;
            break;
        case PLANE_Y:
            if(sliceNr >= input->getHeight())
                sliceNr = input->getHeight()-1;
            break;
        case PLANE_Z:
            if(sliceNr >= input->getDepth())
                sliceNr = input->getDepth()-1;
            break;
        }
    }
    unsigned int slicePlaneNr, width, height;
    Vector3f spacing(0,0,0);
    switch(mOrthogonalSlicePlane) {
        case PLANE_X:
            slicePlaneNr = 0;
            width = input->getHeight();
            height = input->getDepth();
            spacing.x() = input->getSpacing().y();
            spacing.y() = input->getSpacing().z();
            break;
        case PLANE_Y:
            slicePlaneNr = 1;
            width = input->getWidth();
            height = input->getDepth();
            spacing.x() = input->getSpacing().x();
            spacing.y() = input->getSpacing().z();
            break;
        case PLANE_Z:
            slicePlaneNr = 2;
            width = input->getWidth();
            height = input->getHeight();
            spacing.x() = input->getSpacing().x();
            spacing.y() = input->getSpacing().y();
            break;
    }

    output->create(width, height, input->getDataType(), input->getNrOfComponents());
    output->setSpacing(spacing);

    OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

	cl::CommandQueue queue = device->getCommandQueue();
	cl::Program program = getOpenCLProgram(device);
	cl::Kernel kernel(program, "orthogonalSlicing");

    kernel.setArg(0, *inputAccess->get3DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, sliceNr);
    kernel.setArg(3, slicePlaneNr);
    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(width, height),
            cl::NullRange
    );

    // TODO set scene graph transformation
}

void ImageSlicer::arbitrarySlicing(Image::pointer input, Image::pointer output) {
	// TODO
}

}
