#pragma once

#include "FAST/ProcessObject.hpp"
#include "AppearanceModel.hpp"
#include "ShapeModel.hpp"

namespace fast {

class Image;
class Mesh;

/**
 * @brief Kalman filter deformable model segmentation
 *
 * Temporal deformable model segmentation using Kalman filter state estimation
 * Supports multiple shape and appearance models
 *
 * Inputs:
 * - 0: Image to segment
 *
 * Outputs:
 * - 0: Mesh segmentation
 * - 1: Mesh displacement
 *
 * @sa ShapeModel AppearanceModel
 * @ingroup segmentation
 */
class FAST_EXPORT KalmanFilterModelSegmentation : public ProcessObject {
	FAST_PROCESS_OBJECT(KalmanFilterModelSegmentation)
	public:
        /**
         * @brief Create instance
         * @param shapeModel Shape model to use
         * @param appearanceModel Apperrance model to use
         * @param iterations Iterations per frame
         * @param startIterations Iterations for first frame
         * @return instance
         */
        FAST_CONSTRUCTOR(KalmanFilterModelSegmentation,
                         ShapeModel::pointer, shapeModel,,
                         AppearanceModel::pointer, appearanceModel,,
                         int, iterations, = 5,
                         int, startIterations, = 20
        );
		void setShapeModel(ShapeModel::pointer shapeModel);
		void setAppearanceModel(AppearanceModel::pointer appearanceModel);
		void setIterations(int iterations);
		void setStartIterations(int iterations);
		VectorXf getCurrentState() const;
		DataChannel::pointer getSegmentationOutputPort();
		DataChannel::pointer getDisplacementsOutputPort();
	private:
		KalmanFilterModelSegmentation();
		void execute(); // runs a loop with predict, measure and update
		void predict();
		void estimate(std::shared_ptr<Image> image);
		std::shared_ptr<Mesh> getDisplacementVectors(std::shared_ptr<Image> image);

		AppearanceModel::pointer mAppearanceModel;
		ShapeModel::pointer mShapeModel;

		VectorXf mCurrentState;
		VectorXf mPreviousState;
		VectorXf mDefaultState;
		VectorXf mPredictedState;

		MatrixXf mCurrentCovariance;
		MatrixXf mPreviousCovariance;
		MatrixXf mPredictedCovariance;

		bool mInitialized;
		bool mFirstExecute;
		bool mOutputDisplacements;

		int mIterations;
		int mStartIterations;
};

} // end namespace fast
