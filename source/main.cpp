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

    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_0.mhd");
    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setInput(importer->getOutput());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->runMainLoop();


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
    importer->setFilename(std::string(FAST_ROOT_DIR)+"TestData/lena.jpg");
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

    // Example of displaying an image on screen using ImageRenderer (2D) and SimpleWindow


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

	// Example of using VolumeRenderer (3D) and SimpleWindow
	MetaImageImporter::pointer mhdImporter = MetaImageImporter::New();
    mhdImporter->setFilename("skull.mhd");
	VolumeRenderer::pointer VolumeRenderer = VolumeRenderer::New();
    VolumeRenderer->setInput(mhdImporter->getOutput());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->setMaximumFramerate(25);
    window->addRenderer(VolumeRenderer);
    window->runMainLoop();
	*/
	

    /*
    // Example of streaming 2D images
    ImageStreamer::pointer streamer = ImageStreamer::New();
    streamer->setFilenameFormat("test_#.jpg");
    GaussianSmoothingFilter::pointer filter2 = GaussianSmoothingFilter::New();
    filter2->setInput(streamer->getOutput());
    filter2->setMaskSize(7);
    filter2->setStandardDeviation(10);
    DynamicImage::pointer dynamicImage = filter2->getOutput();
    // Call update 4 times
    int i = 4;
    while(--i) {
        dynamicImage->update();
    }
    */


    /*
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat("/home/smistad/US-Acq_01_20140320T105851/US-Acq_01_20140320T105851_cxOpenCV_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    GaussianSmoothingFilter::pointer filter4 = GaussianSmoothingFilter::New();
    filter4->setInput(mhdStreamer->getOutput());
    mhdStreamer->getOutput()->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    //filter4->setDevice(Host::New());
    filter4->setMaskSize(3);
    filter4->setStandardDeviation(10);
    filter4->enableRuntimeMeasurements();
    DynamicImage::pointer asd = filter4->getOutput();
    asd->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInput(filter4->getOutput());
    renderer->enableRuntimeMeasurements();
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->runMainLoop();
    filter4->getRuntime()->print();
    renderer->getRuntime()->print();
    */
	/*
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR) + "TestData/US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    GaussianSmoothingFilter::pointer filter4 = GaussianSmoothingFilter::New();
    filter4->setInput(mhdStreamer->getOutput());
    filter4->setMaskSize(3);
    filter4->setStandardDeviation(10);
    filter4->enableRuntimeMeasurements();
    DynamicImage::pointer asd = filter4->getOutput();

    SliceRenderer::pointer renderer2 = SliceRenderer::New();
    renderer2->setInput(filter4->getOutput());
    renderer2->enableRuntimeMeasurements();
    SimpleWindow::pointer window2 = SimpleWindow::New();
    window2->addRenderer(renderer2);
    window2->runMainLoop();

    filter4->getRuntime()->print();
    renderer2->getRuntime()->print();
	*/

}
