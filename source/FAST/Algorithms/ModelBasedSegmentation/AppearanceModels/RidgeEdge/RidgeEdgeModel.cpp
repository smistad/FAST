#include "RidgeEdgeModel.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Algorithms/ModelBasedSegmentation/Shape.hpp"
#include <boost/shared_array.hpp>

namespace fast {

RidgeEdgeModel::RidgeEdgeModel() {
	mMinimumDepth = 0;
	mLineLength = 0;
	mLineSampleSpacing = 0;
	mIntensityDifferenceThreshold = 20;
    std::vector<float> sizes = {1, 1.5, 2, 2.5, 3, 3.5, 4};
	mSizes = sizes;
}

typedef struct DetectedEdge {
    int edgeIndex;
    float uncertainty;
} DetectedEdge;

inline DetectedEdge findEdge(
        std::vector<float> intensityProfile, const float intensityThreshold, std::vector<int> sizes) {
    // Pre calculate partial sum
    const int line_length = intensityProfile.size();
    boost::shared_array<float> sum_k(new float[line_length]());
    float totalSum = 0.0f;
    for(int k = 0; k < line_length; ++k) {
        if(k == 0) {
            sum_k[k] = intensityProfile[0];
        }else{
            sum_k[k] = sum_k[k-1] + intensityProfile[k];
        }
        totalSum += intensityProfile[k];
    }

    float bestScore = std::numeric_limits<float>::max();
    int bestK = -1;
    float bestHeightDifference = 0;
	for(int size : sizes) {
		for(int k = 1; k < line_length - size; ++k) {
			float score = 0.0f;
			// A 1
			float average1 = sum_k[k] / (k+1);
			for(int t = 0; t <= k; ++t) {
				score += fabs(average1 - intensityProfile[t]);
			}

			// A 2
			float average2 = (sum_k[k+size] - sum_k[k]) / (size);
			for(int t = k+1; t <= k+size; ++t) {
				score += fabs(average2 - intensityProfile[t]);
			}

			// A 3
			float average3 = (sum_k[line_length-1] - sum_k[k+size]) / (line_length - 1 - k - size);
			for(int t = k + size + 1; t < line_length; ++t) {
				score += fabs(average3 - intensityProfile[t]);
			}
			if(score < bestScore) {
				bestScore = score;
				bestK = k;
				float weight1 = (float)(k) / (line_length - size);
				float weight2 = (float)(line_length - k - size) / (line_length - size);
                bestHeightDifference = average2 - (average1*weight1 + average2*weight2);
			}
		}
	}

    DetectedEdge edge;


    if(bestHeightDifference < intensityThreshold) {
        edge.edgeIndex = -1;
    } else {
        edge.edgeIndex = bestK;
        edge.uncertainty = 1.0f/fabs(bestHeightDifference);
    }

    return edge;
}

inline bool isInBounds(Image::pointer image, const Vector4f& position) {
    return position(0) >= 0 && position(1) >= 0 && position(2) >= 0 &&
        position(0) < image->getWidth() && position(1) < image->getHeight() && position(2) < image->getDepth();
}

inline float getValue(void* pixelPointer, Image::pointer image, const Vector4f& position) {

    uint index = (int)position(0)+(int)position(1)*image->getWidth()+(int)position(2)*image->getWidth()*image->getHeight();
    float value;
    switch(image->getDataType()) {
        // This macro creates a case statement for each data type and sets FAST_TYPE to the correct C++ data type
        fastSwitchTypeMacro(value = ((FAST_TYPE*)pixelPointer)[index]);
    }
    return value;
}


std::vector<Measurement> RidgeEdgeModel::getMeasurements(SharedPointer<Image> image, SharedPointer<Shape> shape) {
	if(mLineLength == 0 || mLineSampleSpacing == 0)
		throw Exception("Line length and sample spacing must be given to the RidgeEdgeModel");

	std::vector<Measurement> measurements;
	Mesh::pointer predictedMesh = shape->getMesh();
	MeshAccess::pointer predictedMeshAccess = predictedMesh->getMeshAccess(ACCESS_READ);
	std::vector<MeshVertex> points = predictedMeshAccess->getVertices();

	ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);

	// Convert sizes to pixels, also make sure it is dividible by 2
	std::vector<int> sizesInSteps;
	for(float size : mSizes) {
		// Convert from mm to steps (approx)
		int steps = round(size / mLineSampleSpacing);
		// Make divisible by 2 for convienience
		if(steps % 2 != 0 && steps == 0) {
			steps++;
		}
		sizesInSteps.push_back(steps);
	}

	// For each point on the shape do a line search in the direction of the normal
	// Return set of displacements and uncertainties
	if(image->getDimensions() == 3) {
		AffineTransformation::pointer transformMatrix = SceneGraph::getAffineTransformationFromData(image);
		transformMatrix->scale(image->getSpacing());
		Matrix4f inverseTransformMatrix = transformMatrix->matrix().inverse();

		// Get model scene graph transform
		AffineTransformation::pointer modelTransformation = SceneGraph::getAffineTransformationFromData(shape->getMesh());
		MatrixXf modelTransformMatrix = modelTransformation->affine();

		// Do edge detection for each vertex
		int counter = 0;
		for(int i = 0; i < points.size(); ++i) {
			std::vector<float> intensityProfile;
			unsigned int startPos = 0;
			bool startFound = false;
			for(float d = -mLineLength/2; d < mLineLength/2; d += mLineSampleSpacing) {
				Vector3f position = points[i].getPosition() + points[i].getNormal()*d;
				// Apply model transform
				// TODO the line search normal*d should propably be applied after this transform, so that we know that is correct units?
				position = modelTransformMatrix*position.homogeneous();
				const Vector4f longPosition(position(0), position(1), position(2), 1);
				// Apply image inverse transform to get image voxel position
				const Vector4f positionInt = inverseTransformMatrix*longPosition;
				try {
					const float value = access->getScalar(positionInt.cast<int>());
					if(value > 0) {
						intensityProfile.push_back(value);
						startFound = true;
					} else if(!startFound) {
						startPos++;
					}
				} catch(Exception &e) {
					if(!startFound) {
						startPos++;
					}
				}
			}
			Measurement m;
			m.uncertainty = 1;
			m.displacement = 0;
			if(startFound){
				DetectedEdge edge = findEdge(intensityProfile, mIntensityDifferenceThreshold, sizesInSteps);
				if(edge.edgeIndex != -1) {
					float d = -mLineLength/2.0f + (startPos + edge.edgeIndex)*mLineSampleSpacing;
					const Vector3f position = points[i].getPosition() + points[i].getNormal()*d;
					m.uncertainty = edge.uncertainty;
					const Vector3f normal = points[i].getNormal();
					m.displacement = normal.dot(position-points[i].getPosition());
					counter++;
				}
			}
			measurements.push_back(m);
		}
	} else {
		Vector3f spacing = image->getSpacing();
		// For 2D images
		// For 2D, we probably want to ignore scene graph, and only use spacing.
		// Do edge detection for each vertex
		int counter = 0;
		for(int i = 0; i < points.size(); ++i) {
			std::vector<float> intensityProfile;
			unsigned int startPos = 0;
			bool startFound = false;
			for(float d = -mLineLength/2; d < mLineLength/2; d += mLineSampleSpacing) {
				Vector2f position = points[i].getPosition() + points[i].getNormal()*d;
				const Vector2i pixelPosition(round(position.x() / spacing.x()), round(position.y() / spacing.y()));
				if(position.y() < mMinimumDepth)
					continue;
				try {
					const float value = access->getScalar(pixelPosition);
					if(value > 0) {
						intensityProfile.push_back(value);
						startFound = true;
					} else if(!startFound) {
						startPos++;
					}
				} catch(Exception &e) {
					if(!startFound) {
						startPos++;
					}
				}
			}
			Measurement m;
			m.uncertainty = 1;
			m.displacement = 0;
			if(startFound){
				DetectedEdge edge = findEdge(intensityProfile, mIntensityDifferenceThreshold, sizesInSteps);
				if(edge.edgeIndex != -1) {
					float d = -mLineLength/2.0f + (startPos + edge.edgeIndex)*mLineSampleSpacing;
					const Vector2f position = points[i].getPosition() + points[i].getNormal()*d;
					m.uncertainty = edge.uncertainty;
					const Vector2f normal = points[i].getNormal();
					m.displacement = normal.dot(position-points[i].getPosition());
					counter++;
				}
			}
			measurements.push_back(m);
		}
	}

	return measurements;
}

void RidgeEdgeModel::setRidgeSizes(std::vector<float> sizes) {
	mSizes = sizes;
}

void RidgeEdgeModel::setLineLength(float length) {
	if(length <= 0)
		throw Exception("Length must be > 0");
	mLineLength = length;
}

void RidgeEdgeModel::setLineSampleSpacing(float spacing) {
	if(spacing <= 0)
		throw Exception("Sample spacing must be > 0");
	mLineSampleSpacing = spacing;
}


void RidgeEdgeModel::setIntensityDifferenceThreshold(float threshold) {
	if(threshold <= 0)
		throw Exception("Intensity difference threshold must be > 0");
	mIntensityDifferenceThreshold = threshold;
}

void RidgeEdgeModel::setMinimumDepth(float depth) {
	if(depth < 0)
		throw Exception("Minimum depth must be > 0 in RidgeEdgeModel");
	mMinimumDepth = depth;
}

}
