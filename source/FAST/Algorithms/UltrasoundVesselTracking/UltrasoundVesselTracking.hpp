#ifndef ULTRASOUND_VESSEL_TRACKING_HPP_
#define ULTRASOUND_VESSEL_TRACKING_HPP_

#include "FAST/ProcessObject.hpp"
#include <vector>
#include <deque>
#include "FAST/Data/Image.hpp"

namespace fast {

class UltrasoundVesselTracking : public ProcessObject {
    FAST_OBJECT(UltrasoundVesselTracking)
    public:
        void setDynamicImageInputConnection(ProcessObjectPort port);
        ProcessObjectPort getVesselCrossSectionOutputPort();
        ProcessObjectPort getSegmentationOutputPort();

        void reset();
        void initialize(Vector2f position, float radius, float flattening);

        typedef struct Measurement {
                float uncertainty;
                float displacement;
                VectorXf vector;
        } Measurement;
    private:
        UltrasoundVesselTracking();
        void execute();

        bool mVesselIsDetected;
        bool mTrackingHasStarted;
        bool mCreateOutputSegmentation;

        // Variables for the Kalman Filter
        Vector4f mState_predicted;
        Vector4f mState_t;
        Vector4f mState_t_1;
        Matrix4f mCovariance_predicted;
        Matrix4f mCovariance_t;
        Matrix4f mCovariance_t_1;

        Matrix4f mStateTransitionMatrix1;
        Matrix4f mStateTransitionMatrix2;
        Matrix4f mProcessErrorMatrix;

        float mMinimumRadius, mMaximumRadius;

        // Methods for the Kalman filter
        void predict();
        std::vector<Measurement> doMeasurements(Image::pointer image);
        void updateState(const std::vector<Measurement> measurements);

        void hybridEdgeDetection(const std::vector<float> profile, int* edgeIndex, float* uncertainty);
        void stepEdgeDetection(const std::vector<float> profile, int* edgeIndex, float* uncertainty);
        void maximumGradientEdgeDetection(const std::vector<float> profile, int* edgeIndex, float* uncertainty);

        bool hasTrackingFailed(Image::pointer image);

        std::deque<Vector3f> mCandidatePoints;

};

} // end namespace fast


#endif
