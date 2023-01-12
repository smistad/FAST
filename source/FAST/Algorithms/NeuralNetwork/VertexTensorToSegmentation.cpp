#include "VertexTensorToSegmentation.hpp"
#include <utility>
#include <FAST/Data/MeshVertex.hpp>
#include <FAST/Data/Mesh.hpp>
#include <FAST/Algorithms/MeshToSegmentation/MeshToSegmentation.hpp>
#include <FAST/Data/Image.hpp>
#include "FAST/Data/Tensor.hpp"

namespace fast {

VertexTensorToSegmentation::VertexTensorToSegmentation() {
    createInputPort(0);
    createOutputPort(0);
    createOutputPort(1);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NeuralNetwork/VertexTensorToSegmentation.cl");
}

VertexTensorToSegmentation::VertexTensorToSegmentation(Connections connections, int width, int height) : VertexTensorToSegmentation() {
    m_connections = std::move(connections);
    m_width = width;
    m_height = height;
}

void VertexTensorToSegmentation::execute() {
    auto tensor = getInputData<Tensor>();
    Vector3f spacing = tensor->getSpacing();
    int width = tensor->getFrameData<int>("network-input-size-x");
    int height = tensor->getFrameData<int>("network-input-size-y");

    auto access = tensor->getAccess(ACCESS_READ);
    auto shape = access->getShape();
    auto tensorData = access->getData<2>();

    std::vector<MeshVertex> vertices;
    for(int i = 0; i < shape[1]; ++i) {
        // FIXME assuming here that width and height is the same as the input to the neural network which may not be true.
        vertices.push_back(MeshVertex(Vector3f(
                tensorData( 0, i)*width*spacing.x(),
                tensorData( 1, i)*height*spacing.y(),
                0.0f
        )));
    }

    int width2 = width;
    int height2 = height;
    if(m_width > 0 && m_height > 0) {
        width2 = m_width;
        height2 = m_height;
    }

    // Create initial blank segmentation
    auto segmentation = Image::create(width2, height2, TYPE_UINT8, 1);
    segmentation->fill(0);
    segmentation->setSpacing(Vector3f(width*spacing.x()/width2, height*spacing.y()/height2, 1.0f));

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::Kernel kernel(getOpenCLProgram(device), "writeSegmentation");
    auto segAccess = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    kernel.setArg(0, *segAccess->get2DImage());

    std::vector<MeshLine> allLines;
    // For each object, copy it into the segmentation in order
    for(int label = 0; label < m_connections.size(); ++label) {
        std::vector<MeshLine> lines;
        for(auto connection : m_connections[label]) {
            lines.push_back(MeshLine(connection.getEndpoint1(), connection.getEndpoint2()));
        }
        allLines.insert(allLines.end(), lines.begin(), lines.end());
        if(lines.empty())
            continue;
        auto mesh = Mesh::create(vertices, lines);

        auto newSegmentation = MeshToSegmentation::create(Vector3i(width2, height2, 1), segmentation->getSpacing())->connect(mesh)
                ->runAndGetOutputData<Image>();
        auto accessNew = newSegmentation->getOpenCLImageAccess(ACCESS_READ, device);

        kernel.setArg(1, *accessNew->get2DImage());
        kernel.setArg(2, label+1);

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(segmentation->getWidth(), segmentation->getHeight()),
                cl::NullRange
        );
    }
    addOutputData(0, segmentation);
    addOutputData(1, Mesh::create(vertices, allLines));
}

}