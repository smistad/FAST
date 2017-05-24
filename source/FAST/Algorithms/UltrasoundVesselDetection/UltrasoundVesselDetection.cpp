#include "FAST/Data/Image.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Data/PointSet.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp"
#include "FAST/Algorithms/ImageGradient/ImageGradient.hpp"
#include "FAST/Utility.hpp"
#include "FAST/SceneGraph.hpp"
#include <queue>
#include "FAST/Algorithms/UltrasoundVesselDetection/UltrasoundVesselDetection.hpp"
#include "FAST/Algorithms/ImageCropper/ImageCropper.hpp"
#include "FAST/Algorithms/NeuralNetwork/ImageClassifier.hpp"

namespace fast {

    ProcessObjectPort UltrasoundVesselDetection::getOutputSegmentationPort() {
        mCreateSegmentation = true;
        return getOutputPort(0);
    }

    UltrasoundVesselDetection::UltrasoundVesselDetection() {
        createInputPort<Image>(0);
        createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
        createOutputPort<VesselCrossSection>(1, OUTPUT_STATIC, 0, true);
        createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/UltrasoundVesselDetection/UltrasoundVesselDetection.cl");
        mCreateSegmentation = false;

        mClassifier = ImageClassifier::New();
        //std::string modelFile = "/home/smistad/vessel_detection_model/deploy.prototxt";
        //std::string trainingFile = "/home/smistad/vessel_detection_model/snapshot_iter_540.caffemodel";
        //std::string meanFile = "/home/smistad/vessel_detection_model/mean.binaryproto";
        //std::string modelFile = "/home/smistad/vessel_detection_model2/deploy.prototxt";
        //std::string trainingFile = "/home/smistad/vessel_detection_model2/snapshot_iter_960.caffemodel";
        //std::string meanFile = "/home/smistad/vessel_detection_model2/mean.binaryproto";
        //std::string modelFile = "/home/smistad/vessel_detection_model_gaussian/deploy.prototxt";
        //std::string trainingFile = "/home/smistad/vessel_detection_model_gaussian/snapshot_iter_960.caffemodel";
        //std::string meanFile = "/home/smistad/vessel_detection_model_gaussian/mean.binaryproto";
        //std::string modelFile = "/home/smistad/vessel_detection_model_alex_no_relu/deploy.prototxt";
        //std::string trainingFile = "/home/smistad/vessel_detection_model_alex_no_relu/snapshot_iter_630.caffemodel";
        //std::string meanFile = "/home/smistad/vessel_detection_model_alex_no_relu/mean.binaryproto";
        //std::string modelFile = "/home/smistad/vessel_net/deploy.prototxt";
        //std::string trainingFile = "/home/smistad/vessel_net/snapshot_iter_7600.caffemodel";
        //std::string meanFile = "/home/smistad/vessel_net/mean.binaryproto";
        std::string modelFile = "/home/smistad/vessel_net_1/deploy.prototxt";
        std::string trainingFile = "/home/smistad/vessel_net_1/snapshot_iter_4550.caffemodel";
        std::string meanFile = "/home/smistad/vessel_net_1/mean.binaryproto";
        mClassifier->loadNetworkAndWeights(modelFile, trainingFile);
        mClassifier->loadBinaryMeanImage(meanFile);

    }

    struct Candidate {
        float score;
        VesselCrossSection::pointer crossSection;
    };

    class CandidateComparison {
    public:
        bool operator() (const Candidate& lhs, const Candidate& rhs) const {
            return lhs.score < rhs.score;
        }
    };

    void UltrasoundVesselDetection::execute() {
        Image::pointer input = getStaticInputData<Image>();
        if(input->getDimensions() != 2) {
            throw Exception("The UltrasoundVesselDetection algorithm is only for 2D");
        }

        mRuntimeManager->startRegularTimer("ellipse fitting");
        // Create kernel
        OpenCLDevice::pointer device = getMainDevice();
        cl::Program program = getOpenCLProgram(device);
        cl::Kernel kernel(program, "vesselDetection");

        // Run GaussianSmoothing on input
        GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
        filter->setInputData(input);
        filter->setMaskSize(5);
        filter->setStandardDeviation(2);
        filter->update();
        Image::pointer smoothedImage = filter->getOutputData<Image>();

        // Run ImageGradient on input
        ImageGradient::pointer imageGradient = ImageGradient::New();
        imageGradient->setInputConnection(filter->getOutputPort());
        imageGradient->update();
        Image::pointer gradients = imageGradient->getOutputData<Image>();
        OpenCLImageAccess::pointer inputImageAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer imageAccess = smoothedImage->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer gradientAccess = gradients->getOpenCLImageAccess(ACCESS_READ, device);

        // Create output image
        UniquePointer<float[]> zeros(new float[input->getWidth()*input->getHeight()*4]());
        cl::Image2D result = cl::Image2D(
                device->getContext(),
                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE2D, TYPE_FLOAT, 4),
                input->getWidth(), input->getHeight(),
                0,
                zeros.get()
        );

        // Run vessel detection kernel on smoothed image and gradient
        kernel.setArg(0, *inputImageAccess->get2DImage());
        kernel.setArg(1, *gradientAccess->get2DImage());
        kernel.setArg(2, result);
        kernel.setArg(3, input->getSpacing().x());

        const float minimumDepthInMM = 5;
        const float spacing = input->getSpacing().y();
        //const float maximumDepthInMM = 20;
        const float maximumDepthInMM = input->getHeight()*spacing*0.85;
        int startPosY = round(minimumDepthInMM/spacing);
        int endPosY = round(maximumDepthInMM/spacing);

        // Only process every second pixel
        cl::NDRange globalSize(input->getWidth() / 4, (endPosY-startPosY) / 4);
        cl::NDRange kernelOffset(0, startPosY / 4);
        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                kernelOffset,
                globalSize,
                cl::NullRange
        );

        // Get result back
        UniquePointer<float[]> data(new float[input->getWidth()*input->getHeight()*4]);
        device->getCommandQueue().enqueueReadImage(
                result,
                CL_TRUE,
                createOrigoRegion(),
                createRegion(input->getWidth(),input->getHeight(),1),
                0,0,
                data.get()
        );
        mRuntimeManager->stopRegularTimer("ellipse fitting");
        mRuntimeManager->startRegularTimer("candidate selection");

        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(input);
        Affine3f T = transform->getTransform();
        T.scale(input->getSpacing());
        transform->setTransform(T);

        // Find best ellipses
        std::priority_queue<Candidate, std::vector<Candidate>, CandidateComparison> candidates;
        int startPosY2 = (startPosY / 4)*4;
        for(int x = 0; x < input->getWidth(); x+=4) {
            for(int y = startPosY2; y < endPosY; y+=4) {
                int i = x + y*input->getWidth();

                if(data[i*4] > 1.5) { // If score is higher than a threshold
                    float posY = floor(data[i*4+3]/input->getWidth());
                    float posX = data[i*4+3]-posY*input->getWidth();
                    Vector3f voxelPosition(posX, posY, 0);
                    Vector3f position = transform->multiply(voxelPosition);
                    VesselCrossSection::pointer crossSection = VesselCrossSection::New();
                    crossSection->create(position, voxelPosition.head(2), data[i*4+1], data[i*4+2]*data[i*4+1]);
                    Candidate candidate;
                    candidate.score = data[i*4];
                    candidate.crossSection = crossSection;
                    candidates.push(candidate);

                }
            }
        }

        mCrossSections.clear();
        // Go through all candidates
        while(!candidates.empty()) {
            Candidate next = candidates.top();
            candidates.pop();

            // Check if valid
            bool invalid = false;
            VesselCrossSectionAccess::pointer access2 = next.crossSection->getAccess(ACCESS_READ);
            Vector2f candidateImageCenter = access2->getImageCenterPosition();
            for(VesselCrossSection::pointer crossSection : mCrossSections) {
                VesselCrossSectionAccess::pointer access = crossSection->getAccess(ACCESS_READ);
                Vector2f imageCenter = access->getImageCenterPosition();
                float majorRadius = access->getMajorRadius() + access->getMajorRadius()*0.5f;
                float minorRadius = access->getMinorRadius() + access->getMinorRadius()*0.5f;

                // Check if candidate center is inside a previous ellipse
                if(
                        std::pow(imageCenter.x() - candidateImageCenter.x(), 2.0f) / (majorRadius*majorRadius) +
                        std::pow(imageCenter.y() - candidateImageCenter.y(), 2.0f) / (minorRadius*minorRadius) < 1){
                    invalid = true;
                    break;
                }
            }
            if(!invalid) {
                mCrossSections.push_back(next.crossSection);
            }
        }
        std::cout << mCrossSections.size() << " candidate vessels" << std::endl;
        mRuntimeManager->stopRegularTimer("candidate selection");
        mRuntimeManager->startRegularTimer("classifier");


        Vector3ui imageSize = input->getSize();

        // Create sub images and send to classifier
        std::vector<DataObject::pointer> subImages;
        for(int i = 0; i < std::min((int)mCrossSections.size(), 8); ++i) {
            VesselCrossSectionAccess::pointer access = mCrossSections[i]->getAccess(ACCESS_READ);
            Vector2f imageCenter = access->getImageCenterPosition();

            // Radius in pixels
            const float majorRadius = access->getMajorRadius();
            const float minorRadius = access->getMinorRadius();
            const int frameSize = std::max((int)round(majorRadius), 50); // Nr if pixels to include around vessel

            Vector2i offset(
                    round(imageCenter.x() - majorRadius) - frameSize,
                    round(imageCenter.y() - majorRadius) - frameSize
            );
            int size2 = 2*majorRadius + 2*frameSize;
            Vector2i size(
                    size2,
                    size2
            );

            ImageCropper::pointer cropper = ImageCropper::New();
            cropper->setInputData(input);
            cropper->allowOutOfBoundsCropping(true);
            cropper->setOffset(offset);
            cropper->setSize(size);
            cropper->update();
            subImages.push_back(cropper->getOutputData<Image>());
        }
        std::vector<VesselCrossSection::pointer> acceptedVessels;
        if(subImages.size() > 0) {
            mClassifier->setLabels({"Vessel", "Not vessel"});
            mClassifier->setInputData(subImages);
            mClassifier->update();

            std::vector<ImageClassification::pointer> classifierResult = mClassifier->getMultipleOutputData<ImageClassification>(0);
            int i = 0;
            for(ImageClassification::pointer data : classifierResult) {
                ImageClassification::access access = data->getAccess(ACCESS_READ);
                std::map<std::string, float> scores = access->getData();
                if(scores["Vessel"] > 0.9) {
                    acceptedVessels.push_back(mCrossSections.at(i));
                    //VesselCrossSection::pointer newData = addStaticOutputData<VesselCrossSection>(1);
                    //VesselCrossSectionAccess::pointer access = mCrossSections[i]->getAccess(ACCESS_READ);
                    //newData->create(access->getGlobalCenterPosition(), access->getImageCenterPosition(), access->getMajorRadius(), access->getMinorRadius());
                }
                ++i;
            }
        }
        mAcceptedCrossSections = acceptedVessels;
        mRuntimeManager->stopRegularTimer("classifier");

        if(mCreateSegmentation) {
            mRuntimeManager->startRegularTimer("segmentation");
            Segmentation::pointer segmentation = getStaticOutputData<Segmentation>(0);
            segmentation->createFromImage(input);

            OpenCLDevice::pointer device = getMainDevice();

            // Copy contents
            OpenCLImageAccess::pointer writeAccess = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            cl::Image2D* outputData = writeAccess->get2DImage();
            // Create all zero data
            UniquePointer<uchar[]> zeroData(new uchar[input->getWidth()*input->getHeight()]());
            device->getCommandQueue().enqueueWriteImage(
                    *outputData,
                    CL_TRUE,
                    createOrigoRegion(),
                    createRegion(input->getWidth(), input->getHeight(), 1),
                    0, 0,
                    zeroData.get()
            );
            cl::Kernel kernel(program, "createSegmentation");

            for(VesselCrossSection::pointer crossSection : acceptedVessels) {
                VesselCrossSectionAccess::pointer access = crossSection->getAccess(ACCESS_READ);
                Vector2f imageCenter = access->getImageCenterPosition();
                kernel.setArg(0, *outputData);
                kernel.setArg(1, imageCenter.x());
                kernel.setArg(2, imageCenter.y());
                kernel.setArg(3, access->getMajorRadius());
                kernel.setArg(4, access->getMinorRadius());

                device->getCommandQueue().enqueueNDRangeKernel(
                        kernel,
                        cl::NullRange,
                        cl::NDRange(input->getWidth(), input->getHeight()),
                        cl::NullRange
                );
            }
            mRuntimeManager->stopRegularTimer("segmentation");
        }
    }

    std::vector<VesselCrossSection::pointer> UltrasoundVesselDetection::getCrossSections() {
        return mCrossSections;
    }
    std::vector<VesselCrossSection::pointer> UltrasoundVesselDetection::getAcceptedCrossSections() {
        return mAcceptedCrossSections;
    }

} // end namespace fast


