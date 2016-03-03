#include "StepEdgeModel.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Algorithms/ModelBasedSegmentation/Shape.hpp"

namespace fast {

StepEdgeModel::StepEdgeModel() {
	mLineLength = 0;
	mLineSampleSpacing = 0;
}

typedef struct DetectedEdge {
    int edgeIndex;
    float uncertainty;
} DetectedEdge;

inline DetectedEdge findEdge(
        std::vector<float> intensityProfile) {
    // Pre calculate partial sum
    float * sum_k = new float[intensityProfile.size()]();
    float totalSum = 0.0f;
    for(int k = 0; k < intensityProfile.size(); ++k) {
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
    for(int k = 0; k < intensityProfile.size()-1; ++k) {
        float score = 0.0f;
        if(intensityProfile[k] < 20)
            continue;
        for(int t = 0; t <= k; ++t) {
            score += fabs((1.0f/(k+1))*sum_k[k] - intensityProfile[t]);
        }
        for(int t = k+1; t < intensityProfile.size(); ++t) {
            score += fabs((1.0f/(intensityProfile.size()-k))*(totalSum-sum_k[k]) - intensityProfile[t]);
        }
        if(score < bestScore) {
            bestScore = score;
            bestK = k;
            bestHeightDifference = (((1.0/(k+1))*sum_k[bestK] - (1.0f/(intensityProfile.size()-k))*(totalSum-sum_k[bestK])));
        }
    }
    delete[] sum_k;

    DetectedEdge edge;


    // Should be dark inside and bright outside

    if(bestHeightDifference > 0) {
        bestK = -1;

    }

    edge.edgeIndex = bestK;
    edge.uncertainty = 1.0f / fabs(bestHeightDifference);

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

inline float insertValue(void* pixelPointer, Image::pointer image, Vector3i position) {
    uint index = position(0)+position(1)*image->getWidth()+position(2)*image->getWidth()*image->getHeight();
    switch(image->getDataType()) {
        // This macro creates a case statement for each data type and sets FAST_TYPE to the correct C++ data type
        fastSwitchTypeMacro(((FAST_TYPE*)pixelPointer)[index] = 255);
    }
}


std::vector<Measurement> StepEdgeModel::getMeasurements(SharedPointer<Image> image, SharedPointer<Shape> shape) {
	if(mLineLength == 0 || mLineSampleSpacing == 0)
		throw Exception("Line length and sample spacing must be given to the StepEdgeModel");

	// For each point on the shape do a line search in the direction of the normal
	// Return set of displacements and uncertainties
    AffineTransformation::pointer transformMatrix = SceneGraph::getAffineTransformationFromData(image);
    transformMatrix->scale(image->getSpacing());
    Matrix4f inverseTransformMatrix = transformMatrix->matrix().inverse();

    // Get model scene graph transform
    AffineTransformation::pointer modelTransformation = SceneGraph::getAffineTransformationFromData(shape->getMesh());
    MatrixXf modelTransformMatrix = modelTransformation->affine();

    Mesh::pointer predictedMesh = shape->getMesh();
    MeshAccess::pointer predictedMeshAccess = predictedMesh->getMeshAccess(ACCESS_READ);
    std::vector<MeshVertex> points = predictedMeshAccess->getVertices();

   // TODO This takes a lot of time. why??
	ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
	void* pixelPointer = access->get();

	// Do edge detection for each vertex
	std::vector<Measurement> measurements;
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
			if(isInBounds(image, positionInt)) {
				const float value = getValue(pixelPointer, image, positionInt);
				if(value > 0) {
					intensityProfile.push_back(value);
					startFound = true;
				} else if(!startFound) {
					startPos++;
				}
			} else if(!startFound) {
				startPos++;
			}
		}
		Measurement m;
		m.uncertainty = 1;
		m.displacement = 0;
		if(startFound){
			DetectedEdge edge = findEdge(intensityProfile);
			if(edge.edgeIndex != -1) {
				float d = -mLineLength/2.0f + (startPos + edge.edgeIndex)*mLineSampleSpacing;
				const Vector3f position = points[i].getPosition() + points[i].getNormal()*d;
				m.uncertainty = edge.uncertainty;//1.0f/bestHeightDifference;
				const Vector3f normal = points[i].getNormal();
				m.displacement = normal.dot(position-points[i].getPosition());
				counter++;
			}
		}
		measurements.push_back(m);
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

}
