#include "MeshToSegmentation.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Utility.hpp"

namespace fast {

MeshToSegmentation::MeshToSegmentation() {
	createInputPort<Mesh>(0);
	createInputPort<Image>(1);
	createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/MeshToSegmentation/MeshToSegmentation.cl");

	mResolution = Vector3i::Zero();
}

void MeshToSegmentation::setOutputImageResolution(uint x, uint y, uint z) {
	mResolution = Vector3i(x, y, z);
}

void MeshToSegmentation::execute() {
	Mesh::pointer mesh = getStaticInputData<Mesh>(0);
	Image::pointer image = getStaticInputData<Image>(1);

    Segmentation::pointer segmentation = getStaticOutputData<Segmentation>();
	// Initialize output segmentation image and size
    if(mResolution == Vector3i::Zero()) {
        // If resolution is not specified, use input image resolution
		segmentation->createFromImage(image);
	} else {
		// Use specified resolution
        if(image->getDimensions() == 2) {
			segmentation->create(mResolution.x(), mResolution.y(), TYPE_UINT8, 1);
		} else {
			segmentation->create(mResolution.x(), mResolution.y(), mResolution.z(), TYPE_UINT8, 1);
		}

		// Set correct spacing
		float widthInMM = image->getSpacing().x()*image->getWidth();
		float heightInMM = image->getSpacing().y()*image->getHeight();
		float depthInMM = image->getSpacing().z()*image->getDepth();
		segmentation->setSpacing(widthInMM/mResolution.x(), heightInMM/mResolution.y(), depthInMM/mResolution.z());

		SceneGraph::setParentNode(segmentation, image);
	}

	ExecutionDevice::pointer mainDevice = getMainDevice();

	if(mainDevice->isHost()) {
		throw Exception("Not implemented");
	}

	OpenCLDevice::pointer device = mainDevice;

	// TODO image and mesh scene graph has to be taken into account

    MeshOpenCLAccess::pointer meshAccess = mesh->getOpenCLAccess(ACCESS_READ, device);
    reportInfo() << "Got mesh opencl access" << reportEnd();
	cl::Program program = getOpenCLProgram(device);
	cl::CommandQueue queue = device->getCommandQueue();
	if(image->getDimensions() == 2) {
		cl::Kernel kernel(program, "mesh_to_segmentation_2d");
		OpenCLImageAccess::pointer outputAccess = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(0, *meshAccess->getCoordinatesBuffer());
		kernel.setArg(1, *meshAccess->getLineBuffer());
        kernel.setArg(2, mesh->getNrOfLines());
		kernel.setArg(3, *outputAccess->get2DImage());
		kernel.setArg(4, segmentation->getSpacing().x());
		kernel.setArg(5, segmentation->getSpacing().y());
		kernel.setArg(6, (uchar)mLabel);
		queue.enqueueNDRangeKernel(
				kernel,
				cl::NullRange,
				cl::NDRange(segmentation->getWidth(), segmentation->getHeight()),
				cl::NullRange
		);
	} else {
		cl::Kernel kernel(program, "mesh_to_segmentation_3d");
		OpenCLBufferAccess::pointer outputAccess = segmentation->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(0, *meshAccess->getCoordinatesBuffer());
		kernel.setArg(1, *meshAccess->getTriangleBuffer());
		kernel.setArg(2, mesh->getNrOfTriangles());
		kernel.setArg(3, *outputAccess->get());
		kernel.setArg(4, segmentation->getSpacing().x());
		kernel.setArg(5, segmentation->getSpacing().y());
		kernel.setArg(6, segmentation->getSpacing().z());
        kernel.setArg(7, (uchar)mLabel);
		queue.enqueueNDRangeKernel(
				kernel,
				cl::NullRange,
				cl::NDRange(segmentation->getWidth(), segmentation->getHeight(), segmentation->getDepth()),
				cl::NullRange
		);
	}
    queue.finish();
}

}
