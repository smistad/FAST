#include "ImageFileStreamer.hpp"
#include "MeshRenderer.hpp"
#include "catch.hpp"
#include "MetaImageImporter.hpp"
#include "GaussianSmoothingFilter.hpp"
#include "SliceRenderer.hpp"
#include "SurfaceExtraction.hpp"
#include "ImageRenderer.hpp"
#include "SimpleWindow.hpp"
#include "DeviceManager.hpp"
#include "SeededRegionGrowing.hpp"
#include "Skeletonization.hpp"
#include "ImageImporter.hpp"
#include "ImageRenderer.hpp"
#include "BinaryThresholding.hpp"
#include "VTKPointSetFileImporter.hpp"
#include "IterativeClosestPoint.hpp"
#include "PointRenderer.hpp"
#include "SceneGraph.hpp"

using namespace fast;

TEST_CASE("Pipeline A (static)", "[fast][benchmark][visual]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"/US-3Dt/US-3Dt_50.mhd");
    importer->enableRuntimeMeasurements();

    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->enableRuntimeMeasurements();
    filter->setInputConnection(importer->getOutputPort());
    filter->setMaskSize(5);
    filter->setStandardDeviation(2.0);

    SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
    extractor->enableRuntimeMeasurements();
    extractor->setInputConnection(filter->getOutputPort());
    extractor->setThreshold(200);

    MeshRenderer::pointer renderer = MeshRenderer::New();
    renderer->enableRuntimeMeasurements();
    renderer->addInputConnection(extractor->getOutputPort());

    SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
    sliceRenderer->setInputConnection(filter->getOutputPort());
    sliceRenderer->setSlicePlane(PLANE_X);

    SimpleWindow::pointer window = SimpleWindow::New();
    window->getView()->enableRuntimeMeasurements();
    window->addRenderer(renderer);
    window->addRenderer(sliceRenderer);
    window->setTimeout(2*1000); // timeout after 2 seconds
    window->start();

    importer->getRuntime()->print();
    filter->getRuntime()->print();
    extractor->getRuntime()->print();
    renderer->getRuntime()->print();
    window->getView()->getRuntime("draw")->print();
    float total = importer->getRuntime()->getSum() +
            filter->getRuntime()->getSum() +
            extractor->getRuntime()->getSum() +
            renderer->getRuntime()->getSum() +
            window->getView()->getRuntime("draw")->getAverage();
    std::cout << "Total runtime was: " << total << std::endl;
}

TEST_CASE("Pipeline A (dynamic)", "[fast][benchmark][visual]") {
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"/US-3Dt/US-3Dt_#.mhd");
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    streamer->enableRuntimeMeasurements();

    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setInputConnection(streamer->getOutputPort());
    filter->setMaskSize(5);
    filter->setStandardDeviation(2.0);
    filter->enableRuntimeMeasurements();

    SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
    extractor->setInputConnection(filter->getOutputPort());
    extractor->setThreshold(200);
    extractor->enableRuntimeMeasurements();

    MeshRenderer::pointer renderer = MeshRenderer::New();
    renderer->addInputConnection(extractor->getOutputPort());
    renderer->enableRuntimeMeasurements();

    SimpleWindow::pointer window = SimpleWindow::New();
    window->getView()->enableRuntimeMeasurements();
    window->addRenderer(renderer);
    window->setTimeout(15*1000); // timeout after 10 seconds
    window->start();

    streamer->getRuntime()->print();
    filter->getRuntime()->print();
    extractor->getRuntime()->print();
    renderer->getRuntime()->print();
    window->getView()->getRuntime("draw")->print();
    float total = streamer->getRuntime()->getSum() +
		filter->getRuntime()->getSum() +
		extractor->getRuntime()->getSum() +
		renderer->getRuntime()->getSum() +
            window->getView()->getRuntime("draw")->getAverage();
	total = total / 84; // number of frames
    std::cout << "Average runtime was: " << total << std::endl << std::endl;
}

TEST_CASE("Pipeline B", "[fast][benchmark][visual]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "CT-Abdomen.mhd");
    importer->enableRuntimeMeasurements();

    SeededRegionGrowing::pointer segmentation = SeededRegionGrowing::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->addSeedPoint(223,282,387);
    segmentation->addSeedPoint(251,314,148);
    /*
    segmentation->addSeedPoint(231,281,313);
    segmentation->addSeedPoint(260,284,187);
    segmentation->addSeedPoint(370,290,111);
    segmentation->addSeedPoint(134,293,111);
    */
    segmentation->setIntensityRange(150, 5000);
    segmentation->enableRuntimeMeasurements();

    SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
    extraction->setInputConnection(segmentation->getOutputPort());
    extraction->enableRuntimeMeasurements();

    /*
    VTKSurfaceFileExporter::pointer exporter = VTKSurfaceFileExporter::New();
    exporter->setFilename("bones.vtk");
    exporter->setInputConnection(extraction->getOutputPort());
    exporter->update();
    */

    MeshRenderer::pointer surfaceRenderer = MeshRenderer::New();
    surfaceRenderer->addInputConnection(extraction->getOutputPort());
    surfaceRenderer->enableRuntimeMeasurements();

    SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
    sliceRenderer->setInputConnection(importer->getOutputPort());
    sliceRenderer->setIntensityWindow(1000);
    sliceRenderer->setIntensityLevel(0);
    sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(surfaceRenderer);
    window->getView()->enableRuntimeMeasurements();
    window->addRenderer(sliceRenderer);
    window->setTimeout(2*1000); // timeout after 2 seconds
    window->start();
    importer->getRuntime()->print();
    segmentation->getRuntime()->print();
    extraction->getRuntime()->print();
    surfaceRenderer->getRuntime()->print();
    sliceRenderer->getRuntime()->print();
    window->getView()->getRuntime("draw")->print();

    float total = importer->getRuntime()->getSum()+
                segmentation->getRuntime()->getSum()+
                extraction->getRuntime()->getSum()+
                surfaceRenderer->getRuntime()->getSum() +
                sliceRenderer->getRuntime()->getSum() +
            window->getView()->getRuntime("draw")->getAverage();
    std::cout << "Total runtime was: " << total << std::endl;
}

TEST_CASE("Pipeline C", "[fast][benchmark][visual]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "retina.png");
    importer->enableRuntimeMeasurements();

    BinaryThresholding::pointer thresholding = BinaryThresholding::New();
    thresholding->setInputConnection(importer->getOutputPort());
    thresholding->setLowerThreshold(0.5);
    thresholding->enableRuntimeMeasurements();

    Skeletonization::pointer skeletonization = Skeletonization::New();
    skeletonization->setInputConnection(thresholding->getOutputPort());
    skeletonization->enableRuntimeMeasurements();

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(skeletonization->getOutputPort());
    renderer->setIntensityWindow(1);
    renderer->setIntensityLevel(0.5);
    renderer->enableRuntimeMeasurements();
    SimpleWindow::pointer window = SimpleWindow::New();
    window->getView()->enableRuntimeMeasurements();
    window->addRenderer(renderer);
    window->setTimeout(2000);
    window->start();
    importer->getRuntime()->print();
    thresholding->getRuntime()->print();
    skeletonization->getRuntime()->print();
    renderer->getRuntime()->print();
    window->getView()->getRuntime("draw")->print();
    float total = importer->getRuntime()->getSum() +
            thresholding->getRuntime()->getSum() +
            skeletonization->getRuntime()->getSum() +
            renderer->getRuntime()->getSum() +
            window->getView()->getRuntime("draw")->getAverage();
    std::cout << "Total runtime was: " << total << std::endl;
}

TEST_CASE("Pipeline D", "[fast][benchmark][visual]") {
    VTKPointSetFileImporter::pointer importerA = VTKPointSetFileImporter::New();
    importerA->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");
    importerA->enableRuntimeMeasurements();
    VTKPointSetFileImporter::pointer importerB = VTKPointSetFileImporter::New();
    importerB->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");
    importerB->enableRuntimeMeasurements();

    // Apply a transformation to B surface
    Eigen::Transform<float, 3, Eigen::Affine> transform = Eigen::Transform<float,3,Eigen::Affine>::Identity();
    transform.translate(Vector3f(0.01, 0, 0.01));
    Matrix3f R;
    R = Eigen::AngleAxisf(0.5, Vector3f::UnitX())
    * Eigen::AngleAxisf(0, Vector3f::UnitY())
    * Eigen::AngleAxisf(0, Vector3f::UnitZ());
    transform.rotate(R);
    LinearTransformation transformation;
    transformation.setTransform(transform);
    importerB->update();
    importerB->getOutputData<PointSet>(0)->getSceneGraphNode()->setTransformation(transformation);

    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingPointSetPort(importerA->getOutputPort());
    icp->setFixedPointSetPort(importerB->getOutputPort());
    icp->enableRuntimeMeasurements();
    icp->update();
    std::cout << icp->getOutputTransformation().getTransform().affine() << std::endl;
    importerA->getOutputData<PointSet>(0)->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());
    std::cout << "result: " << std::endl;
    std::cout << icp->getOutputTransformation().getEulerAngles() << std::endl;
    std::cout << icp->getOutputTransformation().getTransform().translation() << std::endl;


    PointRenderer::pointer renderer = PointRenderer::New();
    renderer->addInputConnection(importerA->getOutputPort(), Color::Blue(), 10);
    renderer->addInputConnection(importerB->getOutputPort(), Color::Green(), 5);
    renderer->setDefaultDrawOnTop(true);
    renderer->enableRuntimeMeasurements();

    SimpleWindow::pointer window = SimpleWindow::New();
    window->getView()->enableRuntimeMeasurements();
    window->addRenderer(renderer);
    //window->addRenderer(rendererB);
    window->setTimeout(2000);
    window->start();

    std::cout << "Pipeline D" << std::endl << "===================" << std::endl;
    importerA->getRuntime()->print();
    importerB->getRuntime()->print();
    icp->getRuntime()->print();
    renderer->getRuntime()->print();
    window->getView()->getRuntime("draw")->print();
    float total = importerA->getRuntime()->getSum() +
            importerB->getRuntime()->getSum() +
            icp->getRuntime()->getSum() +
            renderer->getRuntime()->getSum() +
            window->getView()->getRuntime("draw")->getAverage();
    std::cout << "Total runtime was: " << total << std::endl;
}
