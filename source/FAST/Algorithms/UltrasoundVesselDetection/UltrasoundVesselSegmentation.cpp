#include "UltrasoundVesselSegmentation.hpp"
#include "UltrasoundVesselDetection.hpp"
#include "FAST/Algorithms/ModelBasedSegmentation/ShapeModels/Ellipse/EllipseModel.hpp"
#include "FAST/Algorithms/ModelBasedSegmentation/AppearanceModels/StepEdge/StepEdgeModel.hpp"
#include "FAST/Algorithms/ModelBasedSegmentation/KalmanFilter.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Utility.hpp"


namespace fast {

    UltrasoundVesselSegmentation::UltrasoundVesselSegmentation() {
        createInputPort<Image>(0);
        createOutputPort<Segmentation>(0);
        mDetector = UltrasoundVesselDetection::New();
        mFramesToKeep = 10;
        createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/UltrasoundVesselDetection/UltrasoundVesselDetection.cl");
    }

    void UltrasoundVesselSegmentation::execute() {
        Image::pointer image = getInputData<Image>();

        mDetector->setInputData(image);
        mDetector->update();

        float spacing = image->getSpacing().x();
        std::vector<VesselCrossSection::pointer> detectedVessels = mDetector->getAcceptedCrossSections();

        mDetectedVessels.push_back(detectedVessels);
        if(mDetectedVessels.size() == mFramesToKeep)
            mDetectedVessels.pop_front();

        // Are there any new vessels?
        checkForNewVessels(spacing);

        // Run the Kalman filter for each tracked vessel
        trackAllVessels(image);

        createSegmentation(image);
    }

    inline bool isPointInsideEllipse(Vector2f position, VesselCrossSection::pointer crossSection) {
        VesselCrossSectionAccess::pointer access = crossSection->getAccess(ACCESS_READ);
        Vector2f center = access->getImageCenterPosition();
        float a = access->getMajorRadius();
        float b = access->getMinorRadius();
        return (position.x()-center.x())*(position.x()-center.x())/(a*a) + (position.y()-center.y())*(position.y()-center.y())/(b*b) < 1;
    }

    void UltrasoundVesselSegmentation::checkForNewVessels(float spacing) {

        for(TrackedVessel::pointer v : mKalmanFilters) {
            v->framesSinceRefshed++;
        }
        // For each detected vessel, see if it is far from any current tracked vessels
        std::vector<TrackedVessel::pointer> newKalmanFilters;
        for(VesselCrossSection::pointer vessel : mDetectedVessels.back()) {
            VesselCrossSectionAccess::pointer access = vessel->getAccess(ACCESS_READ);
            Vector2f candidatePosition = access->getImageCenterPosition();
            float minorRadius = access->getMinorRadius();
            float majorRadius = access->getMajorRadius();
            bool valid = true;
            for(TrackedVessel::pointer v : mKalmanFilters) {
                Vector4f state = v->filter->getCurrentState();
                Vector2f position = state.head(2) / spacing;
                if((candidatePosition-position).norm() < 2/spacing) {
                    // Refresh
                    v->framesSinceRefshed = 0;
                }
            }
            for(TrackedVessel::pointer v : mKalmanFilters) {
                Vector4f state = v->filter->getCurrentState();
                Vector2f position = state.head(2) / spacing;
                if((candidatePosition-position).norm() < minorRadius) {
                    valid = false;
                    break;
                }
            }

            if(valid) {
                // Check now how many previous frames this vessel has been detected in
                int counter = 0;
                for(std::vector<VesselCrossSection::pointer> detectedVessels : mDetectedVessels) {
                    for(VesselCrossSection::pointer vessel2 : detectedVessels) {
                        if(isPointInsideEllipse(candidatePosition, vessel2)) {
                            counter++;
                        }
                    }
                }

                if(counter > 6) {
                    std::cout << "Accepted new vessel!!!" << std::endl;
                    // Accept it, add a Kalman filter to track it
                    // Add Kalman filter to track new vessel
                    EllipseModel::pointer shapeModel = EllipseModel::New();
                    shapeModel->setInitialState(candidatePosition *spacing, access->getMajorRadius()*spacing, access->getMinorRadius()*spacing);
                    KalmanFilter::pointer segmentation = KalmanFilter::New();
                    StepEdgeModel::pointer appearanceModel = StepEdgeModel::New();
                    appearanceModel->setLineLength(6);
                    appearanceModel->setLineSampleSpacing(6.0/16.0);
                    segmentation->setAppearanceModel(appearanceModel);
                    segmentation->setShapeModel(shapeModel);
                    TrackedVessel::pointer newVessel = TrackedVessel::New();
                    newVessel->filter = segmentation;
                    newVessel->framesSinceRefshed = 0;
                    newKalmanFilters.push_back(newVessel);
                } else {
                    std::cout << "Rejected new vessel because not detected enough times" << std::endl;
                }
            }
        }

        for(TrackedVessel::pointer newFilter : newKalmanFilters)
            mKalmanFilters.push_back(newFilter);

    }

    void UltrasoundVesselSegmentation::trackAllVessels(Image::pointer image) {
        //Reporter::setGlobalReportMethod(Reporter::COUT);
        std::cout << "Nr of vessels being tracked: " << mKalmanFilters.size() << std::endl;
        std::vector<TrackedVessel::pointer> newKalmanFilters;
        for(TrackedVessel::pointer v : mKalmanFilters) {
            v->filter->setInputData(image);
            v->filter->update();

            Vector2f position = v->filter->getCurrentState().head(2) / image->getSpacing().x();
            if(position.x() < 0 || position.x() >= image->getWidth() || position.y() < 0 || position.y() >= image->getHeight()) {
                // Vessel has dissappeared out of image, drop it
            } else {
                newKalmanFilters.push_back(v);
            }
        }

        // Check if any tracked vessels can be merged
        std::vector<TrackedVessel::pointer> newKalmanFilters2;
        for(int i = 0; i < newKalmanFilters.size(); ++i) {
            bool keep = true;
            Vector2f position = newKalmanFilters[i]->filter->getCurrentState().head(2);

            for(int j = 0; j < i; ++j) {
                Vector2f position2 = newKalmanFilters[j]->filter->getCurrentState().head(2);
                if((position-position2).norm() < 2) { // if two vessels have centers closer than 2 mm, merge them
                    keep = false;
                    std::cout << "Merging two vessels" << std::endl;
                    break;
                }
            }
            if(newKalmanFilters[i]->framesSinceRefshed > 8) {
                std::cout << "Removing a vessel due to too long since refresh" << std::endl;
                keep = false;
            }
            if(keep)
                newKalmanFilters2.push_back(newKalmanFilters[i]);
        }
        mKalmanFilters = newKalmanFilters2;
    }

    void UltrasoundVesselSegmentation::createSegmentation(Image::pointer image) {
        Segmentation::pointer segmentation = getOutputData<Segmentation>(0);
        segmentation->createFromImage(image);
        OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        cl::Program program = getOpenCLProgram(device);

        // Copy contents
        OpenCLImageAccess::pointer writeAccess = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        cl::Image2D* outputData = writeAccess->get2DImage();
        // Create all zero data
        std::unique_ptr<uchar[]> zeroData(new uchar[segmentation->getWidth()*segmentation->getHeight()]());
        device->getCommandQueue().enqueueWriteImage(
                *outputData,
                CL_TRUE,
                createOrigoRegion(),
                createRegion(segmentation->getWidth(), segmentation->getHeight(), 1),
                0, 0,
                zeroData.get()
        );
        cl::Kernel kernel(program, "createSegmentation");
        for(TrackedVessel::pointer v : mKalmanFilters) {
            Vector4f state = v->filter->getCurrentState() / image->getSpacing().x();

            kernel.setArg(0, *outputData);
            kernel.setArg(1, state.x());
            kernel.setArg(2, state.y());
            kernel.setArg(3, state(2));
            kernel.setArg(4, state(3));

            device->getCommandQueue().enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(segmentation->getWidth(), segmentation->getHeight()),
                    cl::NullRange
            );
        }
    }

}

