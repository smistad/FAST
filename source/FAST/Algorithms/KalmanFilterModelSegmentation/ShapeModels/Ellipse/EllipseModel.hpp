#pragma once

#include "FAST/Algorithms/KalmanFilterModelSegmentation/ShapeModel.hpp"

namespace fast {

/**
 * @brief Ellipse shape model
 *
 * Used in the Kalman filter deformable model segmentation algorithm
 *
 * @sa KalmanFilter
 */
class FAST_EXPORT EllipseModel : public ShapeModel {
	FAST_OBJECT_V4(EllipseModel)
	public:
        /**
         * @brief Create instance
         * @param position center position of ellipse
         * @param majorRadius major radius
         * @param MinorRadius minor radius
         * @return instance
         */
        FAST_CONSTRUCTOR(EllipseModel,
                         Vector2f, position,,
                         float, majorRadius,,
                         float, MinorRadius,
        );
		Shape::pointer getShape(VectorXf state);
		MatrixXf getStateTransitionMatrix1();
		MatrixXf getStateTransitionMatrix2();
		MatrixXf getStateTransitionMatrix3();
		MatrixXf getProcessErrorMatrix();
		std::vector<MatrixXf> getMeasurementVectors(VectorXf state, Shape::pointer shape);
		/**
		 * Set initial state in mm
		 */
		void setInitialState(Vector2f position, float majorRadius, float minorRadius);
		VectorXf getInitialState(std::shared_ptr<Image> image);
		VectorXf restrictState(VectorXf state);
	private:
		Matrix4f mStateTransitionMatrix1;
		Matrix4f mStateTransitionMatrix2;
		Matrix4f mStateTransitionMatrix3;
		Matrix4f mProcessErrorMatrix;
		int mNrOfNodes;

		VectorXf mInitialState;
};

}