#include "MeshToSegmentation.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Segmentation.hpp"

namespace fast {

MeshToSegmentation::MeshToSegmentation() {
	createInputPort<Mesh>(0);
	createInputPort<Image>(1);
	createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
}

void MeshToSegmentation::execute() {
	Mesh::pointer mesh = getStaticInputData<Mesh>(0);
	Image::pointer image = getStaticInputData<Image>(1);

	Segmentation::pointer segmentation = getStaticOutputData<Segmentation>();
	segmentation->createFromImage(image);

}

}
