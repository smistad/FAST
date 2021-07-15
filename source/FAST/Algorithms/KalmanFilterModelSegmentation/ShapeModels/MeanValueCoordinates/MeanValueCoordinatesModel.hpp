#pragma once

#include "FAST/Algorithms/KalmanFilterModelSegmentation/ShapeModel.hpp"
#include "FAST/Algorithms/KalmanFilterModelSegmentation/Shape.hpp"
#include "FAST/Data/Mesh.hpp"


namespace fast {

class Image;

/**
 * @brief Mean value coordinates shape model
 *
 * Used in Kalman filter deformable model segmentation
 *
 * @sa KalmanFilter CardianlSplineModel EllipseModel
 */
class FAST_EXPORT MeanValueCoordinatesModel : public ShapeModel {
	FAST_OBJECT_V4(MeanValueCoordinatesModel)
	public:
        FAST_CONSTRUCTOR(MeanValueCoordinatesModel,
            std::string, surfaceMeshFilename,,
            std::string, controlMeshFilename,,
            float, globalProcessError, = 0.01f,
            float, localProcessError, = 0.000001f
        )
        FAST_CONSTRUCTOR(MeanValueCoordinatesModel,
             Mesh::pointer, surfaceMesh,,
             Mesh::pointer, controlMesh,,
             float, globalProcessError, = 0.01f,
             float, localProcessError, = 0.000001f
        )
		Shape::pointer getShape(VectorXf state);
		MatrixXf getStateTransitionMatrix1();
		MatrixXf getStateTransitionMatrix2();
		MatrixXf getStateTransitionMatrix3();
		MatrixXf getProcessErrorMatrix();
		VectorXf getInitialState(std::shared_ptr<Image> image);
		std::vector<MatrixXf> getMeasurementVectors(VectorXf state, Shape::pointer shape);
		void initializeShapeToImageCenter();
		void setInitialScaling(float x, float y, float z);
		void setInitialTranslation(float x, float y, float z);
		void setLocalProcessError(float error);
		void setGlobalProcessError(float error);
	private:
        void loadMeshes(std::string surfaceMeshFilename, std::string controlMeshFilename);
        void loadMeshes(Mesh::pointer surfaceMesh, Mesh::pointer controlMesh);
		VectorXf getState(Vector3f translation, Vector3f scale, Vector3f rotation);
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
		std::unordered_map<uint, std::vector<uint>> mModelVertexTrianglesMap;
		std::unordered_map<uint, std::vector<uint>> mControlVertexTrianglesMap;
		Vector3f mCentroid;

		std::unique_ptr<float[]> mNormalizedWeights;
		std::unique_ptr<float[]> mNormalizedWeightsPerNode;

		uint mStateSize;
		MatrixXf mA1;
		MatrixXf mA2;
		MatrixXf mA3;
		MatrixXf mProcessErrorMatrix;

		bool mInitializeShapeToImageCenter;
		Vector3f mInitialScaling;
		Vector3f mInitialTranslation;
		float mLocalProcessError;
		float mGlobalProcessError;

};

} // end namespace fast
