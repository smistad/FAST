#include "StepEdgeModel.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Algorithms/ModelBasedSegmentation/Shape.hpp"


namespace fast {

StepEdgeModel::StepEdgeModel() {
	mMinimumDepth = 0;
	mLineLength = 0;
	mLineSampleSpacing = 0;
	mIntensityDifferenceThreshold = 20;
	mEdgeType = EDGE_TYPE_ANY;
}

void StepEdgeModel::setEdgeType(EdgeType type) {
	mEdgeType = type;
}

typedef struct DetectedEdge {
    int edgeIndex;
    float uncertainty;
} DetectedEdge;

inline DetectedEdge findEdge(
        std::vector<float> intensityProfile, const float intensityThreshold, StepEdgeModel::EdgeType type) {
    // Pre calculate partial sum
    const int size = intensityProfile.size();
    UniquePointer<float[]> sum_k(new float[size]());
    float totalSum = 0.0f;
    for(int k = 0; k < size; ++k) {
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
    for(int k = 0; k < size-1; ++k) {
        float score = 0.0f;
        for(int t = 0; t <= k; ++t) {
            score += fabs((1.0f/(k+1))*sum_k[k] - intensityProfile[t]);
        }
        for(int t = k+1; t < size; ++t) {
            score += fabs((1.0f/(size-k-1))*(totalSum-sum_k[k]) - intensityProfile[t]);
        }
        if(score < bestScore) {
            bestScore = score;
            bestK = k;
            bestHeightDifference = (((1.0/(k+1))*sum_k[k] - (1.0f/(size-k-1))*(totalSum-sum_k[k])));
        }
    }

    DetectedEdge edge;


    if(type == StepEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE && bestHeightDifference >= 0) {
        edge.edgeIndex = -1;
    } else if(type == StepEdgeModel::EDGE_TYPE_WHITE_INSIDE_BLACK_OUTSIDE && bestHeightDifference < 0) {
        edge.edgeIndex = -1;
    } else if(fabs(bestHeightDifference) < intensityThreshold) {
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


std::vector<Measurement> StepEdgeModel::getMeasurements(SharedPointer<Image> image, SharedPointer<Shape> shape, ExecutionDevice::pointer device) {
	if(mLineLength == 0 || mLineSampleSpacing == 0)
		throw Exception("Line length and sample spacing must be given to the StepEdgeModel");

	std::vector<Measurement> measurements;
	Mesh::pointer predictedMesh = shape->getMesh();
	MeshAccess::pointer predictedMeshAccess = predictedMesh->getMeshAccess(ACCESS_READ);
	std::vector<MeshVertex> points = predictedMeshAccess->getVertices();

	ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);

	// For each point on the shape do a line search in the direction of the normal
	// Return set of displacements and uncertainties
	if(image->getDimensions() == 3) {
		AffineTransformation::pointer transformMatrix = SceneGraph::getAffineTransformationFromData(image);
		Matrix4f inverseTransformMatrix = transformMatrix->getTransform().scale(image->getSpacing()).matrix().inverse();

		// Get model scene graph transform
		AffineTransformation::pointer modelTransformation = SceneGraph::getAffineTransformationFromData(shape->getMesh());
		MatrixXf modelTransformMatrix = modelTransformation->getTransform().affine();

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
				DetectedEdge edge = findEdge(intensityProfile, mIntensityDifferenceThreshold, mEdgeType);
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
				Vector2f position = points[i].getPosition().head(2) + points[i].getNormal().head(2)*d;
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
				DetectedEdge edge = findEdge(intensityProfile, mIntensityDifferenceThreshold, mEdgeType);
				if(edge.edgeIndex != -1) {
					float d = -mLineLength/2.0f + (startPos + edge.edgeIndex)*mLineSampleSpacing;
					const Vector2f position = points[i].getPosition().head(2) + points[i].getNormal().head(2)*d;
					m.uncertainty = edge.uncertainty;
					const Vector2f normal = points[i].getNormal().head(2);
					m.displacement = normal.dot(position-points[i].getPosition().head(2));
					counter++;
				}
			}
			measurements.push_back(m);
		}
	}

	return measurements;
}

void StepEdgeModel::setLineLength(float length) {
	if(length <= 0)
		throw Exception("Length must be > 0");
	mLineLength = length;
}

void StepEdgeModel::setLineSampleSpacing(float spacing) {
	if(spacing <= 0)
		throw Exception("Sample spacing must be > 0");
	mLineSampleSpacing = spacing;
}


void StepEdgeModel::setIntensityDifferenceThreshold(float threshold) {
	if(threshold <= 0)
		throw Exception("Intensity difference threshold must be > 0");
	mIntensityDifferenceThreshold = threshold;
}

void StepEdgeModel::setMinimumDepth(float depth) {
	if(depth < 0)
		throw Exception("Minimum depth must be > 0 in StepEdgeModel");
	mMinimumDepth = depth;
}

}
