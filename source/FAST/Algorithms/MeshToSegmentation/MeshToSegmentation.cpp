#include "MeshToSegmentation.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Segmentation.hpp"

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

	// TODO need to get OpenCL buffer access to the mesh somehow

	cl::Program program = getOpenCLProgram(device);
	cl::CommandQueue queue = device->getCommandQueue();
	if(two_dim_data) {
		cl::Kernel kernel(program, "mesh_to_segmentation_2d");
		OpenCLImageAccess::pointer outputAccess = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(0, );
		kernel.setArg(1, *outputAccess->get2DImage());
		queue.enqueueNDRangeKernel(
				kernel,
				cl::NullRange,
				createRegion(segmentation->getSize()),
				cl::NullRange
		);
	} else {
		cl::Kernel kernel(program, "mesh_to_segmentation_3d");
		OpenCLBufferAccess::pointer outputAccess = segmentation->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(0, );
		kernel.setArg(1, *outputAccess->get());
		queue.enqueueNDRangeKernel(
				kernel,
				cl::NullRange,
				createRegion(segmentation->getSize()),
				cl::NullRange
		);
	}
}

}
