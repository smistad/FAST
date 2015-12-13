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
#include "VolumeRenderer.hpp"
#include "ColorTransferFunction.hpp"
#include "OpacityTransferFunction.hpp"

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
    window->setTimeout(15*1000); // timeout after 3 seconds
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


TEST_CASE("SliceRenderer Static Single", "[fast][benchmark][visualization][slice][static][single]")
{

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Static Single:" << std::endl;
	fputs("\n\n\nSliceRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}

TEST_CASE("SliceRenderer Dynamic Single", "[fast][benchmark][visualization][slice][dynamic][single]")
{

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Y);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nSliceRenderer Dynamic Single:\n", pFile);
	std::cout << "\nImport Time:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time :\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("SliceRenderer Static Multi", "[fast][benchmark][visualization][slice][static][multi]")
{
	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();


	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Static Multi:" << std::endl;
	fputs("\n\n\nSliceRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);
	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time :\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}


TEST_CASE("SliceRenderer Dynamic Multi", "[fast][benchmark][visualization][slice][dynamic][multi]")
{
	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();


	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nSliceRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);
	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time :\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}

TEST_CASE("MeshRenderer Static Single", "[fast][benchmark][visualization][mesh][static][single]")
{
	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("nMeshRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Dynamic Single", "[fast][benchmark][visualization][mesh][dynamic][single]")
{
	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Static Multi", "[fast][benchmark][visualization][mesh][static][multi]")
{
	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterStatic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();


	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Dynamic Multi", "[fast][benchmark][visualization][mesh][dynamic][multi]")
{
	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterDynamic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("VolumeRenderer Static Single", "[fast][benchmark][visualization][volume][static][single]")
{
	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("VolumeRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nVolumeRenderer Static Single:" << std::endl;
	fputs("\n\n\nVolumeRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("VolumeRenderer Dynamic Single", "[fast][benchmark][visualization][volume][dynamic][single]")
{
	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if (executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("VolumeRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nVolumeRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nVolumeRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("VolumeRenderer Static Multi", "[fast][benchmark][visualization][volume][static][multi]")
{
	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("VolumeRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nVolumeRenderer Static Multi:" << std::endl;
	fputs("\n\n\nVolumeRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("VolumeRenderer Dynamic Multi", "[fast][benchmark][visualization][volume][dynamic][multi]")
{
	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum())	executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("VolumeRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nVolumeRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nVolumeRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time:" << std::endl;
	fputs("\nRendering Time:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("SliceRenderer Static Single + VolumeRenderer Static Single", "[fast][benchmark][visualization][slicevolume][slicestaticsingle][volumestaticsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Static Single + VolumeRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Static Single + VolumeRenderer Static Single:" << std::endl;
	fputs("\n\n\nSliceRenderer Static Single + VolumeRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);
	
	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("SliceRenderer Static Multi + VolumeRenderer Static Single", "[fast][benchmark][visualization][slicevolume][slicestaticmulti][volumestaticsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();


	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Static Multi + VolumeRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Static Multi + VolumeRenderer Static Single:" << std::endl;
	fputs("\n\n\nSliceRenderer Static Multi + VolumeRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("SliceRenderer Static Single + VolumeRenderer Static Multi", "[fast][benchmark][visualization][slicevolume][slicestaticsingle][volumestaticmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();


	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Static Single + VolumeRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Static Single + VolumeRenderer Static Multi:" << std::endl;
	fputs("\n\n\nSliceRenderer Static Single + VolumeRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("SliceRenderer Static Multi + VolumeRenderer Static Multi", "[fast][benchmark][visualization][slicevolume][slicestaticmulti][volumestaticmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();


	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Y);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Static Multi + VolumeRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Static Multi + VolumeRenderer Static Multi:" << std::endl;
	fputs("\n\n\nSliceRenderer Static Multi + VolumeRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
//------------------------
TEST_CASE("SliceRenderer Dynamic Single + VolumeRenderer Static Single", "[fast][benchmark][visualization][slicevolume][slicedynamicsingle][volumestaticsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Dynamic Single + VolumeRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Dynamic Single + VolumeRenderer Static Single:" << std::endl;
	fputs("\n\n\nSliceRenderer Dynamic Single + VolumeRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("SliceRenderer Dynamic Multi + VolumeRenderer Static Single", "[fast][benchmark][visualization][slicevolume][slicedynamicmulti][volumestaticsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();


	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Dynamic Multi + VolumeRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Dynamic Multi + VolumeRenderer Static Single:" << std::endl;
	fputs("\n\n\nSliceRenderer Dynamic Multi + VolumeRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("SliceRenderer Dynamic Single + VolumeRenderer Static Multi", "[fast][benchmark][visualization][slicevolume][slicedynamicsingle][volumestaticmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Dynamic Single + VolumeRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Dynamic Single + VolumeRenderer Static Multi:" << std::endl;
	fputs("\n\n\nSliceRenderer Dynamic Single + VolumeRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("SliceRenderer Dynamic Multi + VolumeRenderer Static Multi", "[fast][benchmark][visualization][slicevolume][slicedynamicmulti][volumestaticmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();


	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Dynamic Multi + VolumeRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Dynamic Multi + VolumeRenderer Static Multi:" << std::endl;
	fputs("\n\n\nSliceRenderer Dynamic Multi + VolumeRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
//-------------------------------

TEST_CASE("SliceRenderer Static Single + VolumeRenderer Dynamic Single", "[fast][benchmark][visualization][slicevolume][slicestaticsingle][volumedynamicsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Static Single + VolumeRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Static Single + VolumeRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nSliceRenderer Static Single + VolumeRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}

TEST_CASE("SliceRenderer Static Multi + VolumeRenderer Dynamic Single", "[fast][benchmark][visualization][slicevolume][slicestaticmulti][volumedynamicsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Y);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Static Multi + VolumeRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Static Multi + VolumeRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nSliceRenderer Static Multi + VolumeRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("SliceRenderer Static Single + VolumeRenderer Dynamic Multi", "[fast][benchmark][visualization][slicevolume][slicestaticsingle][volumedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Static Single + VolumeRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Static Single + VolumeRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nSliceRenderer Static Single + VolumeRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("SliceRenderer Static Multi + VolumeRenderer Dynamic Multi", "[fast][benchmark][visualization][slicevolume][slicestaticmulti][volumedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Y);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Static Multi + VolumeRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Static Multi + VolumeRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nSliceRenderer Static Multi + VolumeRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}
//-----------------


TEST_CASE("SliceRenderer Dynamic Single + VolumeRenderer Dynamic Single", "[fast][benchmark][visualization][slicevolume][slicedynamicsingle][volumedynamicsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Dynamic Single + VolumeRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Dynamic Single + VolumeRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nSliceRenderer Dynamic Single + VolumeRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("SliceRenderer Dynamic Multi + VolumeRenderer Dynamic Single", "[fast][benchmark][visualization][slicevolume][slicedynamicmulti][volumedynamicsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Y);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Dynamic Multi + VolumeRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Dynamic Multi + VolumeRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nSliceRenderer Dynamic Multi + VolumeRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("SliceRenderer Dynamic Single + VolumeRenderer Dynamic Multi", "[fast][benchmark][visualization][slicevolume][slicedynamicsingle][volumedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);


	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Dynamic Single + VolumeRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Dynamic Single + VolumeRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nSliceRenderer Dynamic Single + VolumeRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("SliceRenderer Dynamic Multi + VolumeRenderer Dynamic Multi", "[fast][benchmark][visualization][slicevolume][slicedynamicmulti][volumedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Y);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 second
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	//if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("SliceRenderer Dynamic Multi + VolumeRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nSliceRenderer Dynamic Multi + VolumeRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nSliceRenderer Dynamic Multi + VolumeRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRendrer:" << std::endl;
	fputs("\nRendering Time of SliceRendrer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile); 
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

TEST_CASE("MeshRenderer Static Single + VolumeRenderer Static Single", "[fast][benchmark][visualization][meshvolume][meshstaticsingle][volumestaticsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Single + VolumeRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Single + VolumeRenderer Static Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Single + VolumeRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Static Multi + VolumeRenderer Static Single", "[fast][benchmark][visualization][meshvolume][meshstaticmulti][volumestaticsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterStatic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();


	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Multi + VolumeRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Multi + VolumeRenderer Static Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Multi + VolumeRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRenderer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Static Single + VolumeRenderer Static Multi", "[fast][benchmark][visualization][meshvolume][meshstaticsingle][volumestaticmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();


	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Single + VolumeRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Single + VolumeRenderer Static Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Single + VolumeRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRenderer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Static Multi + VolumeRenderer Static Multi", "[fast][benchmark][visualization][meshvolume][meshstaticmulti][volumestaticmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();


	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterStatic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Multi + VolumeRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Multi + VolumeRenderer Static Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Multi + VolumeRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
//------------------------
TEST_CASE("MeshRenderer Dynamic Single + VolumeRenderer Static Single", "[fast][benchmark][visualization][meshvolume][meshdynamicsingle][volumestaticsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();


	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Single + VolumeRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Single + VolumeRenderer Static Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Single + VolumeRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Dynamic Multi + VolumeRenderer Static Single", "[fast][benchmark][visualization][meshvolume][meshdynamicmulti][volumestaticsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterDynamic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Multi + VolumeRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Multi + VolumeRenderer Static Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Multi + VolumeRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Dynamic Single + VolumeRenderer Static Multi", "[fast][benchmark][visualization][meshvolume][meshdynamicsingle][volumestaticmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Single + VolumeRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Single + VolumeRenderer Static Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Single + VolumeRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Dynamic Multi + VolumeRenderer Static Multi", "[fast][benchmark][visualization][meshvolume][meshdynamicmulti][volumestaticmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterDynamic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Multi + VolumeRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Multi + VolumeRenderer Static Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Multi + VolumeRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
//-------------------------------

TEST_CASE("MeshRenderer Static Single + VolumeRenderer Dynamic Single", "[fast][benchmark][visualization][meshvolume][meshstaticsingle][volumedynamicsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();


	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Single + VolumeRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Single + VolumeRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Single + VolumeRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}

TEST_CASE("MeshRenderer Static Multi + VolumeRenderer Dynamic Single", "[fast][benchmark][visualization][meshvolume][meshstaticmulti][volumedynamicsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterStatic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Multi + VolumeRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Multi + VolumeRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Multi + VolumeRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Static Single + VolumeRenderer Dynamic Multi", "[fast][benchmark][visualization][meshvolume][meshstaticsingle][volumedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Single + VolumeRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Single + VolumeRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Single + VolumeRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Static Multi + VolumeRenderer Dynamic Multi", "[fast][benchmark][visualization][meshvolume][meshstaticmulti][volumedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterStatic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Multi + VolumeRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Multi + VolumeRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Multi + VolumeRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}
//-----------------


TEST_CASE("MeshRenderer Dynamic Single + VolumeRenderer Dynamic Single", "[fast][benchmark][visualization][meshvolume][meshdynamicsingle][volumedynamicsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Single + VolumeRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Single + VolumeRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Single + VolumeRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Dynamic Multi + VolumeRenderer Dynamic Single", "[fast][benchmark][visualization][meshvolume][meshdynamicmulti][volumedynamicsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterDynamic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Multi + VolumeRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Multi + VolumeRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Multi + VolumeRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Dynamic Single + VolumeRenderer Dynamic Multi", "[fast][benchmark][visualization][meshvolume][meshdynamicsingle][volumedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);


	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Single + VolumeRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Single + VolumeRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Single + VolumeRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Dynamic Multi + VolumeRenderer Dynamic Multi", "[fast][benchmark][visualization][meshvolume][meshdynamicmulti][volumedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterDynamic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(volumeRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	//if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Multi + VolumeRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Multi + VolumeRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Multi + VolumeRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

TEST_CASE("MeshRenderer Static Single + SliceRenderer Static Single", "[fast][benchmark][visualization][meshslice][meshstaticsingle][slicestaticsingle]")
{

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Single + SliceRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Single + SliceRenderer Static Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Single + SliceRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Static Multi + SliceRenderer Static Single", "[fast][benchmark][visualization][meshslice][meshstaticmulti][slicestaticsingle]")
{

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterStatic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Multi + SliceRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Multi + SliceRenderer Static Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Multi + SliceRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRenderer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Static Single + SliceRenderer Static Multi", "[fast][benchmark][visualization][meshslice][meshstaticsingle][slicestaticmulti]")
{

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();


	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Single + SliceRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Single + SliceRenderer Static Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Single + SliceRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRenderer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Static Multi + SliceRenderer Static Multi", "[fast][benchmark][visualization][meshslice][meshstaticmulti][slicestaticmulti]")
{

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();


	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterStatic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Multi + SliceRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Multi + SliceRenderer Static Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Multi + SliceRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
//------------------------
TEST_CASE("MeshRenderer Dynamic Single + SliceRenderer Static Single", "[fast][benchmark][visualization][meshslice][meshdynamicsingle][slicestaticsingle]")
{

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();


	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Single + SliceRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Single + SliceRenderer Static Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Single + SliceRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Dynamic Multi + SliceRenderer Static Single", "[fast][benchmark][visualization][meshslice][meshdynamicmulti][slicestaticsingle]")
{

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterDynamic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Multi + SliceRenderer Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Multi + SliceRenderer Static Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Multi + SliceRenderer Static Single:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Dynamic Single + SliceRenderer Static Multi", "[fast][benchmark][visualization][meshslice][meshdynamicsingle][slicestaticmulti]")
{

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Single + SliceRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Single + SliceRenderer Static Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Single + SliceRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Dynamic Multi + SliceRenderer Static Multi", "[fast][benchmark][visualization][meshslice][meshdynamicmulti][slicestaticmulti]")
{

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterDynamic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Multi + SliceRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Multi + SliceRenderer Static Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Multi + SliceRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
//-------------------------------

TEST_CASE("MeshRenderer Static Single + SliceRenderer Dynamic Single", "[fast][benchmark][visualization][meshslice][meshstaticsingle][slicedynamicsingle]")
{

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();


	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if (executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Single + SliceRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Single + SliceRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Single + SliceRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}

TEST_CASE("MeshRenderer Static Multi + SliceRenderer Dynamic Single", "[fast][benchmark][visualization][meshslice][meshstaticmulti][slicedynamicsingle]")
{

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterStatic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if (executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Multi + SliceRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Multi + SliceRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Multi + SliceRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Static Single + SliceRenderer Dynamic Multi", "[fast][benchmark][visualization][meshslice][meshstaticsingle][slicedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Single + SliceRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Single + SliceRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Single + SliceRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Static:" << std::endl;
	fputs("\nImport Time Static:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Static Multi + SliceRenderer Dynamic Multi", "[fast][benchmark][visualization][meshslice][meshstaticmulti][slicedynamicmulti]")
{

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterStatic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Multi + SliceRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Multi + SliceRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Multi + SliceRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Static 1:" << std::endl;
	fputs("\nImport Time Static 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Static 2:" << std::endl;
	fputs("\nImport Time Static 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}
//-----------------


TEST_CASE("MeshRenderer Dynamic Single + SliceRenderer Dynamic Single", "[fast][benchmark][visualization][meshslice][meshdynamicsingle][slicedynamicsingle]")
{

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->addRenderer(sliceRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Single + SliceRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Single + SliceRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Single + SliceRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Dynamic Multi + SliceRenderer Dynamic Single", "[fast][benchmark][visualization][meshslice][meshdynamicmulti][slicedynamicsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterDynamic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Multi + SliceRenderer Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Multi + SliceRenderer Dynamic Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Multi + SliceRenderer Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Dynamic Single + SliceRenderer Dynamic Multi", "[fast][benchmark][visualization][meshslice][meshdynamicsingle][slicedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);


	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Single + SliceRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Single + SliceRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Single + SliceRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}
TEST_CASE("MeshRenderer Dynamic Multi + SliceRenderer Dynamic Multi", "[fast][benchmark][visualization][meshslice][meshdynamicmulti][slicedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterDynamic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	//if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Multi + SliceRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Multi + SliceRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Multi + SliceRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
TEST_CASE("MeshRenderer Static Single + SliceRenderer Static Single + VolumeRendere Static Single", "[fast][benchmark][visualization][meshslicevolume][meshstaticsingle][slicestaticsingle][volumestaticsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic = MetaImageImporter::New();
	mhdImporterStatic->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->addRenderer(sliceRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Single + SliceRenderer Static Single + VolumeRendere Static Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Single + SliceRenderer Static Single + VolumeRendere Static Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Single + SliceRenderer Static Single + VolumeRendere Static Single:\n", pFile);

	std::cout << "\nImport Time:" << std::endl;
	fputs("\nImport Time:\n", pFile);
	fputs(mhdImporterStatic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Static Multi + SliceRenderer Static Multi + VolumeRenderer Static Multi", "[fast][benchmark][visualization][meshslicevolume][meshstaticmulti][slicestaticmulti][volumestaticmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	MetaImageImporter::pointer mhdImporterStatic1 = MetaImageImporter::New();
	mhdImporterStatic1->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256.mhd");
	mhdImporterStatic1->enableRuntimeMeasurements();

	MetaImageImporter::pointer mhdImporterStatic2 = MetaImageImporter::New();
	mhdImporterStatic2->setFilename(std::string(FAST_TEST_DATA_DIR) + "skull256_2.mhd");
	mhdImporterStatic2->enableRuntimeMeasurements();


	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterStatic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterStatic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterStatic1->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->addInputConnection(mhdImporterStatic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Static Multi + SliceRenderer Static Multi + VolumeRenderer Static Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Static Multi + SliceRenderer Static Multi + VolumeRenderer Static Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Static Multi + SliceRenderer Static Multi + VolumeRenderer Static Multi:\n", pFile);

	std::cout << "\nImport Time 1:" << std::endl;
	fputs("\nImport Time 1:\n", pFile);
	fputs(mhdImporterStatic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time 2:" << std::endl;
	fputs("\nImport Time 2:\n", pFile);
	fputs(mhdImporterStatic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Dynamic Single + SliceRenderer Dynamic Single + Volume Dynamic Single", "[fast][benchmark][visualization][meshslicevolume][meshdynamicsingle][slicedynamicsingle][volumedynamicsingle]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic = ImageFileStreamer::New();
	mhdImporterDynamic->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(meshRenderer);
	window->addRenderer(sliceRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Single + SliceRenderer Dynamic Single + Volume Dynamic Single:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Single + SliceRenderer Dynamic Single + Volume Dynamic Single:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Single + SliceRenderer Dynamic Single + Volume Dynamic Single:\n", pFile);

	std::cout << "\nImport Time Dynamic:" << std::endl;
	fputs("\nImport Time Dynamic:\n", pFile);
	fputs(mhdImporterDynamic->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);

}

TEST_CASE("MeshRenderer Dynamic Multi + SliceRenderer Dynamic Multi + VolumeRenderer Dynamic Mullti", "[fast][benchmark][visualization][meshslicevolume][meshdynamicmulti][slicedynamicmulti][volumedynamicmulti]")
{

	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);

	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(255.0, 1.0);

	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 1.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 1.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);

	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);

	ImageFileStreamer::pointer mhdImporterDynamic1 = ImageFileStreamer::New();
	mhdImporterDynamic1->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic1->enableRuntimeMeasurements();

	ImageFileStreamer::pointer mhdImporterDynamic2 = ImageFileStreamer::New();
	mhdImporterDynamic2->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-3Dt/US-3Dt_#.mhd");
	mhdImporterDynamic2->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor = SurfaceExtraction::New();
	surfaceExtractor->setInputConnection(mhdImporterDynamic1->getOutputPort());
	surfaceExtractor->setThreshold(50);
	surfaceExtractor->enableRuntimeMeasurements();

	SurfaceExtraction::pointer surfaceExtractor2 = SurfaceExtraction::New();
	surfaceExtractor2->setInputConnection(mhdImporterDynamic2->getOutputPort());
	surfaceExtractor2->setThreshold(50);
	surfaceExtractor2->enableRuntimeMeasurements();

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(surfaceExtractor->getOutputPort());
	meshRenderer->addInputConnection(surfaceExtractor2->getOutputPort());
	meshRenderer->enableRuntimeMeasurements();

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	sliceRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Z);
	sliceRenderer->enableRuntimeMeasurements();

	VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
	volumeRenderer->addInputConnection(mhdImporterDynamic1->getOutputPort());
	volumeRenderer->addInputConnection(mhdImporterDynamic2->getOutputPort());
	volumeRenderer->setColorTransferFunction(0, ctf1);
	volumeRenderer->setColorTransferFunction(1, ctf2);
	volumeRenderer->setOpacityTransferFunction(0, otf1);
	volumeRenderer->setOpacityTransferFunction(1, otf2);
	volumeRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->enableRuntimeMeasurements();
	window->setMaximumFramerate(1000);
	window->addRenderer(sliceRenderer);
	window->addRenderer(meshRenderer);
	window->addRenderer(volumeRenderer);
	window->setTimeout(3000); // 3 seconds
	window->start();

	FILE *pFile, *pFileShort;
	pFile = fopen("speedtest.txt", "a");
	pFileShort = fopen("speedtestShort.txt", "a");

	double fps = 0; double executeTime = 0;
	if (sliceRenderer->getRuntime()->getAverage() != sliceRenderer->getRuntime()->getSum())	executeTime += sliceRenderer->getRuntime()->getAverage();
	if (meshRenderer->getRuntime()->getAverage() != meshRenderer->getRuntime()->getSum())	executeTime += meshRenderer->getRuntime()->getAverage();
	if (volumeRenderer->getRuntime()->getAverage() != volumeRenderer->getRuntime()->getSum()) executeTime += volumeRenderer->getRuntime()->getAverage();
	if ( executeTime != 0)
		fps = 1000.0f / ((3000.0f / (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage())) + executeTime);
	else
		fps = (window->getView()->getRuntime("paintGL")->getSum() / window->getView()->getRuntime("paintGL")->getAverage()) / 3.0f;

	fputs(std::string("MeshRenderer Dynamic Multi + SliceRenderer Dynamic Multi+ VolumeRenderer Dynamic Multi:" + (std::to_string(fps)) + "fps\n").c_str(), pFileShort);

	std::cout << "\n\n\nMeshRenderer Dynamic Multi + SliceRenderer Dynamic Multi+ VolumeRenderer Dynamic Multi:" << std::endl;
	fputs("\n\n\nMeshRenderer Dynamic Multi + SliceRenderer Dynamic Multi+ VolumeRenderer Dynamic Multi:\n", pFile);

	std::cout << "\nImport Time Dynamic 1:" << std::endl;
	fputs("\nImport Time Dynamic 1:\n", pFile);
	fputs(mhdImporterDynamic1->getRuntime()->print().c_str(), pFile);

	std::cout << "\nImport Time Dynamic 2:" << std::endl;
	fputs("\nImport Time Dynamic 2:\n", pFile);
	fputs(mhdImporterDynamic2->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of MeshRendrer:" << std::endl;
	fputs("\nRendering Time of MeshRendrer:\n", pFile);
	fputs(meshRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of SliceRenderer:" << std::endl;
	fputs("\nRendering Time of SliceRenderer:\n", pFile);
	fputs(sliceRenderer->getRuntime()->print().c_str(), pFile);

	std::cout << "\nRendering Time of VolumeRenderer:" << std::endl;
	fputs("\nRendering Time of VolumeRenderer:\n", pFile);
	fputs(volumeRenderer->getRuntime()->print().c_str(), pFile);

	fputs(window->getView()->getRuntime("draw")->print().c_str(), pFile);
	fputs(window->getView()->getRuntime("paintGL")->print().c_str(), pFile);

	fclose(pFile); fclose(pFileShort);
}
