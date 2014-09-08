#include "SmartPointers.hpp"
#include "Exception.hpp"
#include "ImageImporter.hpp"
#include "ImageExporter.hpp"
#include "ImageStreamer.hpp"
#include "DeviceManager.hpp"
#include "GaussianSmoothingFilter.hpp"
#include "SimpleWindow.hpp"
#include "ImageRenderer.hpp"
#include "SliceRenderer.hpp"
#include "VolumeRenderer.hpp"
#include "SurfaceRenderer.hpp"
#include "MetaImageImporter.hpp"
#include "MetaImageStreamer.hpp"
#include "MetaImageExporter.hpp"
#include "ColorTransferFunction.hpp"
#include "OpacityTransferFunction.hpp"
#include "SurfaceExtraction.hpp"

using namespace fast;

int main(int argc, char ** argv) {

    // TODO this causes problem for some reason??
    // Get a GPU device and set it as the default device
    //DeviceManager& deviceManager = DeviceManager::getInstance();
    //deviceManager.setDefaultDevice(deviceManager.getOneGPUDevice(true));

    /*
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_0.mhd");
    Image::pointer image = importer->getOutput();
    //MetaImageStreamer::pointer importer = MetaImageStreamer::New();
    //importer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
    //DynamicImage::pointer image = importer->getOutput();
    SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
    extractor->setInput(image);
    extractor->setThreshold(150);
    Surface::pointer surface = extractor->getOutput();
    //extractor->update();

    SurfaceRenderer::pointer surfaceRenderer = SurfaceRenderer::New();
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
    window->runMainLoop();
    */
	/*
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_0.mhd");
    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setSlicePlane(PLANE_Y);
    renderer->setInput(importer->getOutput());
    SimpleWindow::pointer window = SimpleWindow::New();

    window->addRenderer(renderer);
    window->runMainLoop();
	*/
	/*

    ImageImporter::pointer importer2 = ImageImporter::New();
    importer2->setFilename(std::string(FAST_ROOT_DIR)+"TestData/US-2D.jpg");
    ImageRenderer::pointer renderer2 = ImageRenderer::New();
    renderer2->setInput(importer2->getOutput());
    SimpleWindow::pointer window2 = SimpleWindow::New();
    window2->set2DMode();
    window2->addRenderer(renderer2);
    window2->runMainLoop();
	*/


    /*
    MetaImageStreamer::pointer importer = MetaImageStreamer::New();
    importer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
    SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
    extractor->setInput(importer->getOutput());
    extractor->setThreshold(200);
    Surface::pointer surface = extractor->getOutput();
    extractor->update();

    SurfaceRenderer::pointer surfaceRenderer = SurfaceRenderer::New();
    surfaceRenderer->setInput(surface);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(surfaceRenderer);
    window->runMainLoop();
    */

    /*
    // Example of importing, processing and exporting a 2D image
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_ROOT_DIR)+"TestData/US-2D.jpg");
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
    window->runMainLoop();
    */
    
	
/*
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
    SurfaceRenderer::pointer surfaceRenderer = SurfaceRenderer::New();
    surfaceRenderer->setInput(mhdStreamer->getOutput());
    surfaceRenderer->setThreshold(200);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->setMaximumFramerate(25);
    window->addRenderer(surfaceRenderer);
    window->runMainLoop();
*/
	// Example of using VolumeRenderer (3D) and SimpleWindow


//	MetaImageImporter::pointer mhdImporter = MetaImageImporter::New();
//    mhdImporter->setFilename("skull.mhd");
	

	MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");

    SliceRenderer::pointer sRenderer = SliceRenderer::New();
    sRenderer->setSlicePlane(PLANE_Y);
    sRenderer->setInput(mhdStreamer->getOutput());




    //SimpleWindow::pointer window = SimpleWindow::New();
	//window->addRenderer(sRenderer);
    //window->runMainLoop();
	

	


	MetaImageImporter::pointer mhdImporter = MetaImageImporter::New();
    mhdImporter->setFilename("skull.mhd");

	MetaImageImporter::pointer mhdImporter2 = MetaImageImporter::New();
    mhdImporter2->setFilename("stent256.mhd");
	
	
	ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
	ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
	ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);
	ColorTransferFunction::pointer ctf2 = ColorTransferFunction::New();
	ctf2->addRGBPoint(000.0, 0.0, 1.0, 0.0);
	ctf2->addRGBPoint(127.0, 0.0, 1.0, 0.0);
	ctf2->addRGBPoint(255.0, 1.0, 0.0, 0.0);


	OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
	otf1->addAlphaPoint(000.0, 0.0);
	otf1->addAlphaPoint(050.0, 1.0);
	otf1->addAlphaPoint(255.0, 1.0);
	
	OpacityTransferFunction::pointer otf2 = OpacityTransferFunction::New();
	otf2->addAlphaPoint(000.0, 0.0);
	otf2->addAlphaPoint(255.0, 1.0);



	VolumeRenderer::pointer vRenderer = VolumeRenderer::New();
    //VolumeRenderer->addInput(mhdImporter->getOutput());
	vRenderer->addInput(mhdStreamer->getOutput());
	//VolumeRenderer->addInput(mhdImporter2->getOutput());
	
	
	vRenderer->setColorTransferFunction(0, ctf1);
	//VolumeRenderer->setColorTransferFunction(1, ctf2);

	vRenderer->setOpacityTransferFunction(0, otf1);
	//VolumeRenderer->setOpacityTransferFunction(1, otf2);

    vRenderer->enableRuntimeMeasurements();
	SimpleWindow::pointer window = SimpleWindow::New();
    window->setMaximumFramerate(100);
    window->addRenderer(vRenderer);
	window->addRenderer(sRenderer);
    window->runMainLoop();
	vRenderer->getRuntime()->print();
	
	getchar();


}
