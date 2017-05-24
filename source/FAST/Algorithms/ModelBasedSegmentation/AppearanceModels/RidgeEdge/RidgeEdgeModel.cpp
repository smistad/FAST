#include "RidgeEdgeModel.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Algorithms/ModelBasedSegmentation/Shape.hpp"
#include "FAST/Config.hpp"


namespace fast {

RidgeEdgeModel::RidgeEdgeModel() {
	mMinimumDepth = 0;
	mLineLength = 0;
	mLineSampleSpacing = 0;
	mIntensityDifferenceThreshold = 20;
	mRidgeSize = 1;
	mEdgeType = EDGE_TYPE_ANY;
}

typedef struct DetectedEdge {
    int edgeIndex;
    float uncertainty;
} DetectedEdge;

inline DetectedEdge findEdge(
        std::vector<float> intensityProfile, const float intensityThreshold, const int size, const RidgeEdgeModel::EdgeType edgeType) {
    // Pre calculate partial sum
    const int line_length = intensityProfile.size();
    UniquePointer<float[]> sum_k(new float[line_length]());
    float totalSum = 0.0f;
    for(int k = 0; k < line_length; ++k) {
        if(k == 0) {
            sum_k[k] = intensityProfile[0];
        }else{
            sum_k[k] = sum_k[k-1] + intensityProfile[k];
        }
        totalSum += intensityProfile[k];
    }

    float bestScore = std::numeric_limits<float>::min();
    int bestK = -1;
    float intensityDifference = 0;
    for(int k = 4; k < line_length - size; ++k) {
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

		score = fabs(average2 - average1);
		if(score > bestScore) {
            bestScore = score;
            bestK = k;
            intensityDifference = average2 - average1;
        }
    }

    DetectedEdge edge;


	if((edgeType == RidgeEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE && intensityDifference > intensityThreshold) ||
        (edgeType == RidgeEdgeModel::EDGE_TYPE_WHITE_INSIDE_BLACK_OUTSIDE && -intensityDifference > intensityThreshold) ||
        (edgeType == RidgeEdgeModel::EDGE_TYPE_ANY && fabs(intensityDifference) > intensityThreshold)) {
        edge.edgeIndex = bestK;
        edge.uncertainty = 1.0f/fabs(intensityDifference);
    } else {
		edge.edgeIndex = -1;
	}

    return edge;
}


int RidgeEdgeModel::convertRidgeSizeToSamples() {
	const int nrOfSamples = ceil(mLineLength / mLineSampleSpacing);
	int ridgeSizeInSteps = round(mRidgeSize / mLineSampleSpacing);
    if(ridgeSizeInSteps == 0)
        ridgeSizeInSteps = 1;
	if(ridgeSizeInSteps < 6)
		reportWarning() << "Warning only " << ridgeSizeInSteps << " samples are used for detecting ridge. Consider increasing the minimum ridge size or lower the line sample spacing." << reportEnd();
    if(ridgeSizeInSteps >= nrOfSamples)
		throw Exception("Ridge size too large compared to the line length");

	return ridgeSizeInSteps;
}

std::vector<Measurement> RidgeEdgeModel::getMeasurementsOnHost(SharedPointer<Image> image, SharedPointer<Shape> shape) {
	std::vector<Measurement> measurements;
	Mesh::pointer predictedMesh = shape->getMesh();
	MeshAccess::pointer predictedMeshAccess = predictedMesh->getMeshAccess(ACCESS_READ);
	std::vector<MeshVertex> points = predictedMeshAccess->getVertices();

	ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);

    // Convert from mm to steps (approx)
	int ridgeSizeInSteps = convertRidgeSizeToSamples();

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
				DetectedEdge edge = findEdge(intensityProfile, mIntensityDifferenceThreshold, ridgeSizeInSteps, mEdgeType);
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

			// Sample values along line
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

			// Check for edge along the line
			Measurement m;
			m.uncertainty = 1;
			m.displacement = 0;
			if(startFound){
				DetectedEdge edge = findEdge(intensityProfile, mIntensityDifferenceThreshold, ridgeSizeInSteps, mEdgeType);
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

std::vector<Measurement> RidgeEdgeModel::getMeasurementsOnDevice(SharedPointer<Image> image, SharedPointer<Shape> shape, OpenCLDevice::pointer device) {
	Mesh::pointer predictedMesh = shape->getMesh();
	MeshAccess::pointer predictedMeshAccess = predictedMesh->getMeshAccess(ACCESS_READ);
	std::vector<MeshVertex> points = predictedMeshAccess->getVertices();

	cl::CommandQueue queue = device->getCommandQueue();
	UniquePointer<float[]> pointsArray(new float[points.size()*2*2]);
	for(int i = 0; i < points.size(); i++) {
		pointsArray[i*2*2] = points[i].getPosition().x();
		pointsArray[i*2*2+1] = points[i].getPosition().y();
		//std::cout << "normalx: " << points[i].getNormal().x() << std::endl;
		//std::cout << "normaly: " << points[i].getNormal().y() << std::endl;
		pointsArray[i*2*2+2] = points[i].getNormal().x();
		pointsArray[i*2*2+3] = points[i].getNormal().y();
	}

	const int nrOfSamples = ceil(mLineLength / mLineSampleSpacing);

    int ridgeSizeInSteps = convertRidgeSizeToSamples();

	// TODO the read only buffers here can be cached
	cl::Buffer pointsBuffer(
			device->getContext(),
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			points.size()*2*2*sizeof(float),
			pointsArray.get()
	);
	cl::Buffer resultBuffer(
			device->getContext(),
			CL_MEM_WRITE_ONLY,
			points.size()*nrOfSamples*sizeof(float)
	);

	// Create kernel
	int programNr = device->createProgramFromSource(Config::getKernelSourcePath() + "Algorithms/ModelBasedSegmentation/AppearanceModels/RidgeEdge/RidgeEdgeModel.cl");
	cl::Program program = device->getProgram(programNr);
	cl::Kernel kernel(program, "edgeDetection2D");

	const int pointSize = points.size();

	OpenCLImageAccess::pointer access = image->getOpenCLImageAccess(ACCESS_READ, device);

	const float spacingX = image->getSpacing().x();
	const float spacingY = image->getSpacing().y();
	kernel.setArg(0, *access->get2DImage());
	kernel.setArg(1, pointsBuffer);
	kernel.setArg(2, mLineLength);
	kernel.setArg(3, mLineSampleSpacing);
	kernel.setArg(4, sizeof(float)*nrOfSamples, NULL);
	kernel.setArg(5, resultBuffer);
	kernel.setArg(6, spacingX);
	kernel.setArg(7, spacingY);
	kernel.setArg(8, ridgeSizeInSteps);

	// Launch kernel
	queue.enqueueNDRangeKernel(
			kernel,
			cl::NullRange,
			cl::NDRange(nrOfSamples, pointSize),
			cl::NDRange(nrOfSamples, 1)
	);
    queue.finish();


	// Transfer data back
	UniquePointer<float[]> resultArray(new float[pointSize*nrOfSamples]);
	queue.enqueueReadBuffer(resultBuffer, CL_TRUE, 0, pointSize*nrOfSamples*sizeof(float), resultArray.get());

	std::vector<Measurement> measurements;
	for(int i = 0; i < pointSize; ++i) {
		Measurement m;
		m.uncertainty = 1;
		m.displacement = 0;
		// Minimize score
		float bestScore = std::numeric_limits<float>::min();
		float bestUncertainty = 0.0f;
		int edgeIndex = -1;
        for(int sampleNr = 4; sampleNr < nrOfSamples - ridgeSizeInSteps; ++sampleNr) {
            float intensityDifference = resultArray[sampleNr + i*nrOfSamples];
			float score = fabs(intensityDifference);
            if(score > bestScore) {
			   if((mEdgeType == EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE && intensityDifference > mIntensityDifferenceThreshold) ||
                   (mEdgeType == EDGE_TYPE_WHITE_INSIDE_BLACK_OUTSIDE && -intensityDifference > mIntensityDifferenceThreshold) ||
                   (mEdgeType == EDGE_TYPE_ANY && fabs(intensityDifference) > mIntensityDifferenceThreshold)) {
				   bestScore = score;
				   bestUncertainty = 1.0f/fabs(intensityDifference);
				   edgeIndex = sampleNr;
			   }
            }
        }
		if(edgeIndex != -1) {
			//std::cout << edgeIndex << " " << bestScore << " " << bestUncertainty << std::endl;
			float distance = -mLineLength/2.0f + edgeIndex*mLineSampleSpacing;
			const Vector2f position = points[i].getPosition().head(2) + points[i].getNormal().head(2)*distance;
			if(position.y() > mMinimumDepth) {
				m.uncertainty = bestUncertainty;
				const Vector2f normal = points[i].getNormal().head(2);
				m.displacement = normal.dot(position - points[i].getPosition().head(2));
			}
		}
        measurements.push_back(m);
	}

	return measurements;
}

std::vector<Measurement> RidgeEdgeModel::getMeasurements(SharedPointer<Image> image, SharedPointer<Shape> shape, ExecutionDevice::pointer device) {
	if(mLineLength == 0 || mLineSampleSpacing == 0)
		throw Exception("Line length and sample spacing must be given to the RidgeEdgeModel");

	if(device->isHost()) {
		return getMeasurementsOnHost(image, shape);
	} else {
		return getMeasurementsOnHost(image, shape);
		//return getMeasurementsOnDevice(image, shape, device);
	}

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

void RidgeEdgeModel::setMinimumRidgeSize(float sizeInMM) {
	if(sizeInMM <= 0)
		throw Exception("Minimum ridge size must be > 0");

	mRidgeSize = sizeInMM;
}

void RidgeEdgeModel::setEdgeType(EdgeType type) {
	mEdgeType = type;
}

}
