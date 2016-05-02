#include "UltrasoundVesselTracking.hpp"
#include "FAST/Data/PointSet.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Utility.hpp"
#include <boost/shared_array.hpp>
#include <stack>
#include "FAST/Utility.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "VesselCrossSection.hpp"

namespace fast {

void UltrasoundVesselTracking::setDynamicImageInputConnection(
        ProcessObjectPort port) {
    setInputConnection(0, port);
}

ProcessObjectPort UltrasoundVesselTracking::getVesselCrossSectionOutputPort() {
    return getOutputPort(0);
}

ProcessObjectPort UltrasoundVesselTracking::getSegmentationOutputPort() {
    mCreateOutputSegmentation = true;
    return getOutputPort(1);
}

void UltrasoundVesselTracking::reset() {
    mVesselIsDetected = false;
}

UltrasoundVesselTracking::UltrasoundVesselTracking() {
    createInputPort<Image>(0);
    createOutputPort<VesselCrossSection>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOutputPort<Image>(1, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(std::string(ASSISTANT_SOURCE_DIR) + "UltrasoundVesselTracking/UltrasoundVesselTracking.cl");
    mVesselIsDetected = false;
    mTrackingHasStarted = false;
    mCreateOutputSegmentation = false;

    // Set up state transition matrices
    float dampening = 0.5; // 0 is no dampening, 1 is full
    mStateTransitionMatrix1 = Matrix4f::Zero();
    mStateTransitionMatrix1(0,0) = 2 - dampening;
    mStateTransitionMatrix1(1,1) = 2 - dampening;
    mStateTransitionMatrix1(2,2) = 1;//0.5;
    mStateTransitionMatrix1(3,3) = 1;//0.5;
    mStateTransitionMatrix2 = Matrix4f::Zero();
    mStateTransitionMatrix2(0,0) = dampening - 1;
    mStateTransitionMatrix2(1,1) = dampening - 1;
    mStateTransitionMatrix2(2,2) = 0;//0.5;
    mStateTransitionMatrix2(3,3) = 0;//0.5;
    mProcessErrorMatrix = Matrix4f::Zero();
    mProcessErrorMatrix(0,0) = 0.01;
    mProcessErrorMatrix(1,1) = 0.01;
    mProcessErrorMatrix(2,2) = 0.01;
    mProcessErrorMatrix(3,3) = 0.01;
}

void UltrasoundVesselTracking::execute() {
    Image::pointer input = getStaticInputData<Image>();
    mMinimumRadius = 3.5f/input->getSpacing().x();
    mMaximumRadius = 6.0f/input->getSpacing().x();
    AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(input);
    transform->scale(input->getSpacing());

    Vector3f detectedCenterpoint;
    // if no vessel is detected run vessel detection
    if(mVesselIsDetected) {
        mRuntimeManager->startRegularTimer("tracking");

        for(uint j = 0; j < 5; j++) { // Kalman iterations
            // Predict state
            predict();

            // Do measurements
            std::vector<Measurement> measurements = doMeasurements(input);

            // Update state
            updateState(measurements);
        }

        // Convert pixel position to mm position
        detectedCenterpoint = transform->multiply(Vector3f(mState_t(0), mState_t(1), 0));

        if(mState_t.x() < 0 || mState_t.x() >= input->getWidth() || mState_t.y() < 0 || mState_t.y() >= input->getHeight()) {
            mVesselIsDetected = false;
            reportInfo() << "TRACKING DROPPED OUT OF IMAGE" << Reporter::end;
        } else {
            // Check intensity variation inside vessel
            if(hasTrackingFailed(input)) {
                mVesselIsDetected = false;
            }
        }
        mRuntimeManager->stopRegularTimer("tracking");
    }


    // Output the centerpoint
    VesselCrossSection::pointer output = getStaticOutputData<VesselCrossSection>();

    if(mVesselIsDetected) {
        VesselCrossSectionAccess::pointer access = output->getAccess(ACCESS_READ);
        Vector3f spacing = input->getSpacing();
        access->setMajorRadius(mState_t(2)*spacing.x());
        access->setMinorRadius(mState_t(3)*spacing.y());
        access->setGlobalCenterPosition(detectedCenterpoint);
        access->setImageCenterPosition(Vector2f(mState_t(0)*spacing.x(), mState_t(1)*spacing.y()));
    } else {
        // Output empty PointSet if no centerpoint is found
    }

    // Output segmentation if desired
    if(mCreateOutputSegmentation) {
        OpenCLDevice::pointer device = getMainDevice();
        Segmentation::pointer segmentation = getStaticOutputData<Segmentation>(1);
        segmentation->createFromImage(input);

        // Copy contents
        OpenCLImageAccess::pointer writeAccess = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        cl::Image2D* outputData = writeAccess->get2DImage();

        if(mVesselIsDetected) {
            cl::Program program = getOpenCLProgram(device);
            cl::Kernel kernel(program, "createSegmentation");
            float flattening = 1.0f - mState_t(3)/mState_t(2);
            kernel.setArg(0, *outputData);
            kernel.setArg(1, mState_t.x());
            kernel.setArg(2, mState_t.y());
            kernel.setArg(3, mState_t(2));
            kernel.setArg(4, flattening);
            kernel.setArg(5, (uchar)Segmentation::LABEL_ARTERY);

            device->getCommandQueue().enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(input->getWidth(), input->getHeight()),
                    cl::NullRange
            );
        } else {
            // Create all zero data
            boost::shared_array<uchar> zeroData(new uchar[input->getWidth()*input->getHeight()]());
            device->getCommandQueue().enqueueWriteImage(
                    *outputData,
                    CL_TRUE,
                    createOrigoRegion(),
                    createRegion(input->getWidth(), input->getHeight(), 1),
                    0, 0,
                    zeroData.get()
                    );
        }
    }
}

bool UltrasoundVesselTracking::hasTrackingFailed(Image::pointer image) {
    // Use mState_t to collect intensity/gradient data from image

    const uint samples = 32;
    const uint samplePixelSpacing = 4;
    const Vector2f center(mState_t(0), mState_t(1));
    const float targetRadius = mState_t(2); // radius in pixels
    const float flattening = 1 - mState_t(3)/targetRadius;

    ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);

    // Get average lumen intensity
    float averageLumenIntensity = 0.0f;
    uint counterLumen = 0;
    for(float radius = 3; radius < targetRadius; radius += samplePixelSpacing) {
        for(uint sample = 0; sample < samples; sample++) {
            const float alpha = (float)(2.0f*M_PI*sample)/samples;
            Vector2f direction(cos(alpha), sin(alpha)*(1.0f-flattening));
            Vector2f samplePosition = center + radius*direction;
            Vector2i samplePositionInt(round(samplePosition.x()), round(samplePosition.y()));

            // Get intensity
            try {
                float intensity = access->getScalar(samplePositionInt);
                averageLumenIntensity += intensity;
                counterLumen ++;
            } catch(OutOfBoundsException &e) {
            }
        }
    }

    // Get border intensity
    float averageBorderIntensity = 0.0f;
    uint counterBorder = 0;
    for(uint sample = 0; sample < samples; sample++) {
        const float alpha = (float)(2.0f*M_PI*sample)/samples;
        Vector2f direction(cos(alpha), sin(alpha)*(1.0f-flattening));
        Vector2f samplePosition = center + targetRadius*direction;
        Vector2i samplePositionInt(round(samplePosition.x()), round(samplePosition.y()));

        // Get intensity
        try {
            float intensity = access->getScalar(samplePositionInt);
            averageBorderIntensity += intensity;
            counterBorder ++;
        } catch(OutOfBoundsException &e) {
        }
    }

    averageBorderIntensity /= counterBorder;
    averageLumenIntensity /= counterLumen;
    reportInfo() << "Average lumen intensity is: " << averageLumenIntensity << Reporter::end;
    reportInfo() << "Average border intensity is: " << averageBorderIntensity << Reporter::end;

    if(averageBorderIntensity < averageLumenIntensity /*|| averageBorderIntensity < 20*/) {
        return true;
    } else {
        return false;
    }
}

void UltrasoundVesselTracking::initialize(Vector2f position, float radius,
        float flattening) {
    reportInfo() << "INITIALIZING TRACKING AT " << position.transpose() << Reporter::end;
    reportInfo() << "WITH RADIUS " << radius << Reporter::end;
    mState_t(0) = position.x();
    mState_t(1) = position.y();
    mState_t(2) = radius;
    mState_t(3) = radius*(1-flattening);
    mState_t_1 = mState_t;
    mCovariance_t = Matrix4f::Zero();
    mCovariance_t_1 = Matrix4f::Zero();
    mVesselIsDetected = true;
}

void UltrasoundVesselTracking::predict() {
    mState_predicted = mStateTransitionMatrix1*mState_t + mStateTransitionMatrix2*mState_t_1;
    mCovariance_predicted = mStateTransitionMatrix1*mCovariance_t*mStateTransitionMatrix1.transpose() +
            mStateTransitionMatrix2*mCovariance_t_1*mStateTransitionMatrix2.transpose() +
            mStateTransitionMatrix1*mCovariance_t*mStateTransitionMatrix2.transpose() +
            mStateTransitionMatrix2*mCovariance_t_1*mStateTransitionMatrix1.transpose() +
            mProcessErrorMatrix;
}

std::vector<UltrasoundVesselTracking::Measurement> UltrasoundVesselTracking::doMeasurements(Image::pointer image) {
    std::vector<Measurement> measurements;
    uint numberOfMeasurements = 16;
    float radiusStep = 4; // pixels
    float flattening = 1.0f - mState_predicted(3)/mState_predicted(2);
    Vector2f center;
    center.x() = round(mState_predicted(0));
    center.y() = round(mState_predicted(1));

    float predictedRadius = mState_predicted(2);
    float radiusMin = - predictedRadius*0.5f;
    float radiusMax = predictedRadius*0.5f;

    ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);

    // Use predicted state to do measurements in the form of edge detection along the circle
    for(uint measurementNumber = 0; measurementNumber < numberOfMeasurements; measurementNumber++) {
        float alpha = 2.0*M_PI*measurementNumber/numberOfMeasurements;
        Vector2f direction(cos(alpha), (1-flattening)*sin(alpha));
        std::vector<float> profile;
        uint counter = 0;
        int startPosFoundAt = -1;
        Vector2f positionOnCircle = center + direction*predictedRadius;
        Vector2f normal((1-flattening)*predictedRadius*cos(alpha), predictedRadius*sin(alpha));
        normal.normalize();

        for(float radius = radiusMin; radius < radiusMax; radius += radiusStep) {
            Vector2f position = positionOnCircle + radius*normal;
            //reportInfo() << position.transpose() << ", ";
            try {
                Vector2i positionInt(round(position.x()), round(position.y()));
                float intensity = access->getScalar(positionInt);
                //reportInfo() << intensity << " ";
                profile.push_back(intensity);
                if(startPosFoundAt == -1) {
                    startPosFoundAt = counter;
                }
                counter++;
            } catch(OutOfBoundsException &e) {};
        }
        //reportInfo() << Reporter::end;

        if(profile.size() == 0)
            continue;

        int edgeIndex = -1;
        float uncertainty;
        hybridEdgeDetection(profile, &edgeIndex, &uncertainty);
        //reportInfo() << "Edge i: " << edgeIndex << Reporter::end;
        if(edgeIndex >= 0) {
            Vector2f detectedEdgePosition = positionOnCircle + normal*(radiusMin + (startPosFoundAt + edgeIndex)*radiusStep);
            Vector2f predictedEdgePosition = positionOnCircle;
            //reportInfo() << "Detected: " << detectedEdgePosition.transpose() << " Predicted: " << predictedEdgePosition.transpose() << Reporter::end;

            MatrixXf derivativeMatrix = MatrixXf::Zero(2, 4);
            derivativeMatrix.col(0) = Vector2f(1,0);
            derivativeMatrix.col(1) = Vector2f(0,1);
            derivativeMatrix(0,2) = cos(alpha);
            derivativeMatrix(1,3) = sin(alpha);

            Measurement measurement;
            measurement.displacement = normal.dot(detectedEdgePosition - predictedEdgePosition);
            //reportInfo() << "DISPLACEMENT " << measurement.displacement << Reporter::end;
            measurement.uncertainty = uncertainty;
            measurement.vector = normal.transpose()*derivativeMatrix;

            measurements.push_back(measurement);
        }
    }

    //reportInfo() << measurements.size() << " MEASUREMENTS COLLECTED" << Reporter::end;

    return measurements;
}

void UltrasoundVesselTracking::updateState(const std::vector<Measurement> measurements) {

    // Assimilate measurement vectors
    VectorXf HRv = VectorXf::Zero(4);
    MatrixXf HRH = MatrixXf::Zero(4, 4);
    float scalingFactor = 0.1f; // Why is this needed?
    for(uint i = 0; i < measurements.size(); i++) {
        Measurement measurement = measurements[i];
        HRv = HRv + measurement.vector*(1.0f/measurement.uncertainty)*scalingFactor*measurement.displacement;
        HRH = HRH + measurement.vector*(1.0f/measurement.uncertainty)*scalingFactor*measurement.vector.transpose();
    }

    // Update state and covariance
    mCovariance_t_1 = mCovariance_t;
    mCovariance_t = (mCovariance_predicted.inverse() + HRH).inverse();
    mState_t_1 = mState_t;
    mState_t = mState_predicted + mCovariance_t*HRv;

    //reportInfo() << "NEW STATE IS " << mState_t.transpose() << Reporter::end;

    // Restrict radius and flattening
    if(mState_t(2) < mMinimumRadius) {
        mState_t(2) = mMinimumRadius;
    } else if(mState_t(2) > mMaximumRadius) {
        mState_t(2) = mMaximumRadius;
    }

    if(mState_t(3) > mState_t(2)) {
        mState_t(3) = mState_t(2);
    } else if(mState_t(3) < 0.5*mState_t(2)) {
        mState_t(3) = 0.5*mState_t(2);
    }
}

void UltrasoundVesselTracking::hybridEdgeDetection(
        const std::vector<float> profile, int* edgeIndex, float* uncertainty) {
    stepEdgeDetection(profile, edgeIndex, uncertainty);
    if(*edgeIndex == -1) {
        maximumGradientEdgeDetection(profile, edgeIndex, uncertainty);
    }
}

void UltrasoundVesselTracking::stepEdgeDetection(
        const std::vector<float> profile, int* edgeIndex, float* uncertainty) {

    const float differenceThreshold = 20;

    // Pre calculate partial sum
    boost::shared_array<float> sum_k(new float[profile.size()]());
    float totalSum = 0.0f;
    for(int k = 0; k < profile.size(); ++k) {
        if(k == 0) {
            sum_k[k] = profile[0];
        }else{
            sum_k[k] = sum_k[k-1] + profile[k];
        }
        totalSum += profile[k];
    }

    float bestScore = std::numeric_limits<float>::max();
    int bestK = -1;
    float bestHeightDifference = 0;
    for(int k = 0; k < profile.size()-1; ++k) {
        float score = 0.0f;
        for(int t = 0; t <= k; ++t) {
            score += fabs(sum_k[k]/(k+1) - profile[t]);
        }
        for(int t = k+1; t < profile.size(); ++t) {
            score += fabs((totalSum-sum_k[k])/(profile.size()-k) - profile[t]);
        }
        if(score < bestScore) {
            bestScore = score;
            bestK = k;
            bestHeightDifference = (((1.0/(k+1))*sum_k[bestK] - (1.0f/(profile.size()-k))*(totalSum-sum_k[bestK])));
        }
    }

    if(bestHeightDifference >= 0) { // Black inside, white outside
        *edgeIndex = -1;
    } else if(fabs(bestHeightDifference) < differenceThreshold) {
        *edgeIndex = -1;
    } else {
        *edgeIndex = bestK;
        *uncertainty = 1.0f/fabs(bestHeightDifference);
    }
}

void UltrasoundVesselTracking::maximumGradientEdgeDetection(
        const std::vector<float> profile, int* edgeIndex, float* uncertainty) {

    int bestK = -1;
    float currentMax = 0.0;
    for(int k = 0; k < profile.size()-1; k++) {
        if(profile[k+1]-profile[k] > 20) {
            bestK = k;
            currentMax = profile[k+1]-profile[k];
            break;
        }
    }

    *edgeIndex = bestK;
    *uncertainty = 1.0f / currentMax;
}

}
