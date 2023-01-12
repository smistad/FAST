#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/MeshVertex.hpp>

namespace fast {

using Connections = std::vector<std::vector<MeshLine>>;

/**
 * @brief Convert a tensor of vertex positions (graph) to a segmentation
 *
 * A vertex tensor is assumed to have the shape (2, N) consisting of N points with x, y positions.
 * If multiple objects are present, they are drawn in the order specified.
 *
 * Inputs:
 * - 0: Tensor
 *
 * Outputs:
 * - 0: Segmentation Image
 * - 1: Mesh with vertices and lines
 */
class FAST_EXPORT VertexTensorToSegmentation : public ProcessObject {
    FAST_PROCESS_OBJECT(VertexTensorToSegmentation)
    public:
        /**
         * @brief Create instance
         *
         * If multiple objects are provided to the connections param, they are drawn in the order the they are specified.
         * @param connections List of connections for each object
         * @param width specify width of output segmentation. If not specified width of network input that created the tensor is used instead.
         * @param height specify height of output segmentation. If not specified, height of network input that created the tensor is used instead.
         * @return instance
         */
        FAST_CONSTRUCTOR(VertexTensorToSegmentation,
                         Connections, connections,,
                         int, width, = -1,
                         int, height, = -1)
    protected:
        VertexTensorToSegmentation();
        void execute() override;
        Connections m_connections;
        int m_width;
        int m_height;
};

}