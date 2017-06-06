#include "FAST/SmartPointers.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Importers/ImageImporter.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/VolumeRenderer/VolumeRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/VolumeRenderer/ColorTransferFunction.hpp"
#include "FAST/Visualization/VolumeRenderer/OpacityTransferFunction.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"


using namespace fast;

int main(int argc, char ** argv) {

    // TODO this causes problem for some reason??
    // Get a GPU device and set it as the default device
    //DeviceManager* deviceManager = DeviceManager::getInstance();
    //deviceManager.setDefaultDevice(deviceManager->getOneGPUDevice(true));

    /*
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_ROOT_DIR)+"TestData/US/Ball/US-3Dt_0.mhd");
    Image::pointer image = importer->getOutput();
    //ImageFileStreamer::pointer importer = ImageFileStreamer::New();
    //importer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US/Ball/US-3Dt_#.mhd");
    //DynamicImage::pointer image = importer->getOutput();
    SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
    extractor->setInput(image);
    extractor->setThreshold(150);
    Surface::pointer surface = extractor->getOutput();
    //extractor->update();

    TriangleRenderer::pointer surfaceRenderer = TriangleRenderer::New();
    surfaceRenderer->setInput(surface);
    SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
    sliceRenderer->setInput(image);
    SliceRenderer::pointer sliceRenderer2 = SliceRenderer::New();
    sliceRenderer2->setInput(image);
    sliceRenderer2->setSlicePlane(PLANE_X);
    //sliceRenderer2->setSliceToRender(200);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(surfaceRenderer);
    window->addRenderer(sliceRenderer);
    window->addRenderer(sliceRenderer2);
    window->start();
    */
	/*
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_ROOT_DIR)+"TestData/US/Ball/US-3Dt_0.mhd");
    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setSlicePlane(PLANE_Y);
    renderer->setInput(importer->getOutput());
    SimpleWindow::pointer window = SimpleWindow::New();

    window->addRenderer(renderer);
    window->start();
	*/
	/*

    ImageImporter::pointer importer2 = ImageImporter::New();
    importer2->setFilename(std::string(FAST_ROOT_DIR)+"TestData/US/US-2D.jpg");
    ImageRenderer::pointer renderer2 = ImageRenderer::New();
    renderer2->setInput(importer2->getOutput());
    SimpleWindow::pointer window2 = SimpleWindow::New();
    window2->set2DMode();
    window2->addRenderer(renderer2);
    window2->start();
	*/


    /*
    ImageFileStreamer::pointer importer = ImageFileStreamer::New();
    importer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US/Ball/US-3Dt_#.mhd");
    SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
    extractor->setInput(importer->getOutput());
    extractor->setThreshold(200);
    Surface::pointer surface = extractor->getOutput();
    extractor->update();

    TriangleRenderer::pointer surfaceRenderer = TriangleRenderer::New();
    surfaceRenderer->setInput(surface);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(surfaceRenderer);
    window->start();
    */

    /*
    // Example of importing, processing and exporting a 2D image
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_ROOT_DIR)+"TestData/US/US-2D.jpg");
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setInput(importer->getOutput());
    filter->setMaskSize(7);
    filter->setStandardDeviation(10);
    Image::pointer filteredImage = filter->getOutput();

    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename("test.jpg");
    exporter->setInput(filteredImage);
    exporter->update();
    MetaImageExporter::pointer exporter2 = MetaImageExporter::New();
    exporter2->setFilename("test.mhd");
    exporter2->setInput(filteredImage);
    exporter2->update();
	*/
    // Example of displaying an image on screen using ImageRenderer (2D) and SimpleWindow

/*
    SimpleWindow::pointer window = SimpleWindow::New();
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInput(filteredImage);
    window->addRenderer(renderer);
window->setTimeout(10*1000);
    window->start();
    */
    
	
/*
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US/Ball/US-3Dt_#.mhd");
    TriangleRenderer::pointer surfaceRenderer = TriangleRenderer::New();
    surfaceRenderer->setInput(mhdStreamer->getOutput());
    surfaceRenderer->setThreshold(200);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->setMaximumFramerate(25);
    window->addRenderer(surfaceRenderer);
    window->start();
*/
	// Example of using VolumeRenderer (3D) and SimpleWindow


//	MetaImageImporter::pointer mhdImporter = MetaImageImporter::New();
//    mhdImporter->setFilename("skull.mhd");
	

	ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
	mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");

	MetaImageImporter::pointer mhdImporter = MetaImageImporter::New();
	mhdImporter->setFilename(Config::getTestDataPath() + "skull256.mhd");

	MetaImageImporter::pointer mhdImporter2 = MetaImageImporter::New();
	mhdImporter2->setFilename(Config::getTestDataPath() + "v8.mhd");

	
    SliceRenderer::pointer sRenderer = SliceRenderer::New();
    sRenderer->setSlicePlane(PLANE_Y);
    sRenderer->setInputConnection(mhdStreamer->getOutputPort());
	//sRenderer->setInputConnection(mhdImporter->getOutputPort());
	sRenderer->setSliceToRender(120);
	



    //SimpleWindow::pointer window = SimpleWindow::New();
	//window->addRenderer(sRenderer);
    //window->start();
	

	


	
	
	
	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);
	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf2->addRGBPoint(255.0, 0.0, 0.0, 1.0);


	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(200.0, 1.0);
	otf1->addAlphaPoint(255.0, 1.0);
	
	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);



	VolumeRenderer::pointer vRenderer = VolumeRenderer::New();
	vRenderer->addInputConnection(mhdStreamer->getOutputPort());
	//vRenderer->addInputConnection(mhdStreamer->getOutputPort());
	//vRenderer->addInputConnection(mhdStreamer->getOutputPort());
	//vRenderer->addInputConnection(mhdStreamer->getOutputPort());
	//vRenderer->addInputConnection(mhdImporter->getOutputPort());
	//vRenderer->addInputConnection(mhdImporter2->getOutputPort());
	//vRenderer->turnOffTransformations();
	
	vRenderer->setColorTransferFunction(0, ctf1);
	//vRenderer->setColorTransferFunction(1, ctf2);
	//vRenderer->setColorTransferFunction(2, ctf1);

	vRenderer->setOpacityTransferFunction(0, otf1);
	//vRenderer->setOpacityTransferFunction(1, otf2);
	//vRenderer->setOpacityTransferFunction(2, otf1);
	float ut[16]={	1.0, 0.0, 0.0, -100.0,
					0.0, 1.0, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					0.0, 0.0, 0.0, 1.0};
	ut[3] = 0.0f;
	//vRenderer->setUserTransform(1, ut);
	ut[3] = 100.0f;
	//vRenderer->setUserTransform(2, ut);
	
    //vRenderer->enableRuntimeMeasurements();
	SimpleWindow::pointer window = SimpleWindow::New();
    window->setMaximumFramerate(15);
    window->addRenderer(vRenderer);
	window->addRenderer(sRenderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
	//vRenderer->getRuntime()->print();
	
	//getchar(); // Do not commit this as the example is run on all platforms..


}
