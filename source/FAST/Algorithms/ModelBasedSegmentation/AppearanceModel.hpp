#ifndef APPEARANCE_MODEL_HPP
#define APPEARANCE_MODEL_HPP

#include "FAST/Data/DataTypes.hpp"
#include "Shape.hpp"

namespace fast {

class Image;

class AssimilatedMeasurements {
	public:
		// TODO find better names
		MatrixXf HRv;
		VectorXf HRH;
};

/**
 * This is a base class for appearance models.
 * These classes model of an object appears in an image.
 * They are used by the Kalman filter to collect measurements.
 */
class AppearanceModel {
	public:
		typedef SharedPointer<AppearanceModel> pointer;
		virtual AssimilatedMeasurements getMeasurements(SharedPointer<Image> image, Shape shape) = 0;


};

}

#endif
