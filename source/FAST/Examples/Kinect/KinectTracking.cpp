#include <FAST/Streamers/KinectStreamer.hpp>
#include "KinectTracking.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Algorithms/IterativeClosestPoint/IterativeClosestPoint.hpp"
#include <FAST/Exporters/VTKMeshFileExporter.hpp>
#include <QDir>

namespace fast {

KinectTracking::KinectTracking() {

    createInputPort<Image>(0);
    createInputPort<Mesh>(1);

    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOutputPort<Image>(1, OUTPUT_STATIC); // Annotation image
    createOutputPort<Mesh>(2, OUTPUT_STATIC); // Target cloud

    // Create annotation image
    mAnnotationImage = Image::New();
    mAnnotationImage->create(512, 424, TYPE_UINT8, 1);
    mAnnotationImage->fill(0);

    mTargetCloud = Mesh::New();
    mTargetCloud->create(0);
    mTargetCloudExtracted = false;
    getReporter().setReportMethod(Reporter::COUT);
}

void KinectTracking::restart() {
    stopRecording();
    mTargetCloudExtracted = false;
    mAnnotationImage->fill(0);
}

void KinectTracking::startRecording(std::string path) {
    mStoragePath = path;
    mFrameCounter = 0;
    mRecording = true;
}

void KinectTracking::stopRecording() {
    mRecording = false;
}

void KinectTracking::execute() {
    Image::pointer input = getStaticInputData<Image>();
    Mesh::pointer meshInput = getStaticInputData<Mesh>(1);

    // When target cloud has been extracted, run ICP, and output this mesh
    if(mTargetCloudExtracted) {
        reportInfo() << "Running ICP" << reportEnd();
        IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
        icp->enableRuntimeMeasurements();
        icp->setFixedMesh(meshInput);
        icp->setMovingMesh(mTargetCloud);
        icp->setDistanceThreshold(150); // All points further away than 10 cm from the centroid is removed
        //icp->setMinimumErrorChange(0.5);
        icp->setRandomPointSampling(300);
        icp->getReporter().setReportMethod(Reporter::COUT);
        icp->setMaximumNrOfIterations(10);
        icp->update();
        reportInfo() << "Finished ICP in: " << reportEnd();
        icp->getAllRuntimes()->printAll();
        AffineTransformation::pointer currentTransform = mTargetCloud->getSceneGraphNode()->getTransformation();
        AffineTransformation::pointer newTransform = icp->getOutputTransformation();
        mTargetCloud->getSceneGraphNode()->setTransformation(newTransform->multiply(currentTransform));
    }

    if(mRecording) {
        VTKMeshFileExporter::pointer exporter = VTKMeshFileExporter::New();
        exporter->setInputData(meshInput);
        exporter->setWriteNormals(false);
        exporter->setFilename(mStoragePath + std::to_string(mFrameCounter) + ".vtk");
        exporter->update();
        ++mFrameCounter;
    }

    setStaticOutputData<Image>(0, input);
    setStaticOutputData<Image>(1, mAnnotationImage);
    setStaticOutputData<Mesh>(2, mTargetCloud);
    mCurrentCloud = meshInput;
}

void KinectTracking::calculateTargetCloud(KinectStreamer::pointer streamer) {
    std::cout << "Creating target cloud..." << std::endl;
    ImageAccess::pointer access = mAnnotationImage->getImageAccess(ACCESS_READ);
    MeshAccess::pointer meshAccess = mCurrentCloud->getMeshAccess(ACCESS_READ);
    std::vector<MeshVertex> vertices = meshAccess->getVertices();
    std::vector<MeshVertex> outputVertices;
    for(int y = 0; y < mAnnotationImage->getHeight(); ++y) {
        for(int x = 0; x < mAnnotationImage->getWidth(); ++x) {
            try {
                if(access->getScalar(Vector2i(x, y)) == 1) {
                    MeshVertex vertex = streamer->getPoint(x, y);
                    if(!std::isnan(vertex.getPosition().x())) {
                        outputVertices.push_back(vertex);
                    }
                }
            } catch (Exception &e) {

            }
        }
    }

    mTargetCloud = Mesh::New();
    mTargetCloud->create(outputVertices);
    std::cout << "Created target cloud." << std::endl;
    mTargetCloudExtracted = true;
}

void KinectTracking::addLine(Vector2i start, Vector2i end) {
    if(mTargetCloudExtracted)
        return;
    std::cout << "Drawing from: " << start.transpose() << " to " << end.transpose() << std::endl;
    // Draw line in some auxillary image
    ImageAccess::pointer access = mAnnotationImage->getImageAccess(ACCESS_READ_WRITE);
    Vector2f direction = end.cast<float>() - start.cast<float>();
    int length = (end-start).norm();
    int brushSize = 6;
    for(int i = 0; i < length; ++i) {
        float distance = (float)i/length;
        for(int a = -brushSize; a <= brushSize; a++) {
            for(int b = -brushSize; b <= brushSize; b++) {
                Vector2f offset(a, b);
                if(offset.norm() > brushSize)
                    continue;
                Vector2f position = start.cast<float>() + direction*distance + offset;
                try {
                    access->setScalar(position.cast<int>(), 1);
                } catch(Exception &e) {

                }
            }
        }
    }
}

uint KinectTracking::getFramesStored() const {
    return mFrameCounter;
}

bool KinectTracking::isRecording() const {
    return mRecording;
}

SharedPointer<Mesh> KinectTracking::getTargetCloud() const {
    return mTargetCloud;
}

void KinectTracking::setTargetCloud(Mesh::pointer target) {
    mTargetCloud = target;
    mTargetCloudExtracted = true;
}

}