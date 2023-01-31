#include <FAST/Testing.hpp>
#include <FAST/Importers/NIFTIImporter.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Visualization/SliceRenderer/SliceRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

/*
TEST_CASE("NIFT Importer", "[NIFTIImporter][fast]") {
    auto importer = NIFTIImporter::create(Config::getTestDataPath() + "/MRI/MR-Abdomen.nii.gz");
    auto image = importer->runAndGetOutputData<Image>();
    float max = image->calculateMaximumIntensity();
    float min = image->calculateMinimumIntensity();
    std::cout << max << " " << min << std::endl;

    auto renderer = SliceRenderer::create(fast::PLANE_Y)->connect(image);
    renderer->setIntensityLevel((max-min)/2.0f);
    renderer->setIntensityWindow(max-min);
    SimpleWindow2D::create()->connect(renderer)->run();
}*/