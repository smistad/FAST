#include "MeshToSegmentation.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Utility.hpp"

namespace fast {

MeshToSegmentation::MeshToSegmentation() {
	createInputPort<Mesh>(0);
	createInputPort<Image>(1);
	createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
	createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/MeshToSegmentation/MeshToSegmentation.cl");
}

void MeshToSegmentation::execute() {
	Mesh::pointer mesh = getStaticInputData<Mesh>(0);
	Image::pointer image = getStaticInputData<Image>(1);

	Segmentation::pointer segmentation = getStaticOutputData<Segmentation>();
	segmentation->createFromImage(image);

	bool two_dim_data = true;
	if(mesh->getDimensions() == 2 && image->getDimensions() == 2) {
		two_dim_data = true;
	} else if(mesh->getDimensions() == 3 && image->getDimensions() == 3) {
		two_dim_data = false;
	} else {
		throw Exception("Dimensions of mesh and image in mesh to segmentation doesn't match.");
	}

	ExecutionDevice::pointer mainDevice = getMainDevice();

	if(mainDevice->isHost()) {
		throw Exception("Not implemented");
	}

	OpenCLDevice::pointer device = mainDevice;

	// TODO image and mesh scene graph has to be taken into account

	cl::Program program = getOpenCLProgram(device);
	cl::CommandQueue queue = device->getCommandQueue();
	MeshOpenCLAccess::pointer meshAccess = mesh->getOpenCLAccess(ACCESS_READ, device);
	reportInfo() << "Got mesh opencl access" << reportEnd();
	if(two_dim_data) {
		cl::Kernel kernel(program, "mesh_to_segmentation_2d");
		OpenCLImageAccess::pointer outputAccess = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(0, *meshAccess->getCoordinatesBuffer());
		kernel.setArg(1, *meshAccess->getConnectionsBuffer());
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
		kernel.setArg(1, *meshAccess->getConnectionsBuffer());
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
}

}
