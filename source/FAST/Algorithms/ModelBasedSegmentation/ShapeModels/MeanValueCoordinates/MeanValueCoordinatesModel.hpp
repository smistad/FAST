#ifndef MEAN_VALUE_COORDINATES_MODEL_HPP
#define MEAN_VALUE_COORDINATES_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/ShapeModel.hpp"
#include "FAST/Algorithms/ModelBasedSegmentation/Shape.hpp"
#include "FAST/Data/Mesh.hpp"
#include <boost/shared_array.hpp>

namespace fast {

class MeanValueCoordinatesModel : ShapeModel {
	public:
		typedef SharedPointer<MeanValueCoordinatesModel> pointer;
		void loadMeshes(Mesh::pointer surfaceMesh, Mesh::pointer controlMesh);
		Shape::pointer getShape(VectorXf state);
		MatrixXf getStateTransitionMatrix1();
		MatrixXf getStateTransitionMatrix2();
		MatrixXf getStateTransitionMatrix3();
		std::vector<VectorXf> getMeasurementVectors(VectorXf state, Shape::pointer shape);
	private:
		void assertLoadedMeshes();
        void setNormalizedWeight(
                const int vertexNr,
                const int triangleNr,
                const int triangleVertexNr,
                float weight
        );
        float getNormalizedWeight(const int vertexNr, const int triangleNr, const int triangleVertexNr);
        float getNormalizedWeight(const uint vertexNr, const uint i);
        void setNormalizedWeightPerNode(
            const uint vertexNr,
            const uint controlNodeNr
            );
        std::vector<MeshVertex> getDeformedVertices(const std::vector<Vector3f>& displacements);
		std::vector<MeshVertex> getOriginalVertices();

		Mesh::pointer mSurfaceMesh;
		Mesh::pointer mControlMesh;
		Vector3f mCentroid;

		boost::shared_array<float> mNormalizedWeights;
		boost::shared_array<float> mNormalizedWeightsPerNode;

		uint mStateSize;
		MatrixXf mA1;
		MatrixXf mA2;
		MatrixXf mA3;




};

}

#endif
