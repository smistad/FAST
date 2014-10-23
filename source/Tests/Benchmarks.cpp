#include "catch.hpp"
#include "MetaImageImporter.hpp"
#include "MetaImageStreamer.hpp"
#include "GaussianSmoothingFilter.hpp"
#include "SliceRenderer.hpp"
#include "SurfaceRenderer.hpp"
#include "SurfaceExtraction.hpp"
#include "ImageRenderer.hpp"
#include "SimpleWindow.hpp"
#include "DeviceManager.hpp"
#include "SeededRegionGrowing.hpp"

using namespace fast;

TEST_CASE("Pipeline B") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "CT-Abdomen.mhd");
    importer->enableRuntimeMeasurements();

    SeededRegionGrowing::pointer segmentation = SeededRegionGrowing::New();
    segmentation->setInput(importer->getOutput());
    segmentation->addSeedPoint(223,282,387);
    segmentation->addSeedPoint(251,314,148);
    segmentation->setIntensityRange(150, 5000);

    SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
    extraction->setInput(segmentation->getOutput());

    /*
    VTKSurfaceFileExporter::pointer exporter = VTKSurfaceFileExporter::New();
    exporter->setFilename("bones.vtk");
    exporter->setInput(extraction->getOutput());
    exporter->update();
    */

    SurfaceRenderer::pointer surfaceRenderer = SurfaceRenderer::New();
    surfaceRenderer->setInput(extraction->getOutput());

    SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
    sliceRenderer->setInput(importer->getOutput());
    sliceRenderer->setIntensityWindow(1000);
    sliceRenderer->setIntensityLevel(0);

	SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(surfaceRenderer);
    //window->addRenderer(sliceRenderer);
    window->runMainLoop();
    importer->getRuntime()->print();
    segmentation->getRuntime()->print();
    extraction->getRuntime()->print();
    surfaceRenderer->getRuntime()->print();
    sliceRenderer->getRuntime()->print();

    float total = importer->getRuntime()->getSum()+
                segmentation->getRuntime()->getSum()+
                extraction->getRuntime()->getSum()+
                surfaceRenderer->getRuntime()->getSum() +
                sliceRenderer->getRuntime()->getSum();
    std::cout << "Total runtime was: " << total << std::endl;
}
