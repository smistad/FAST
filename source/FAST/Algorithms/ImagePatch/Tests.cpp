#include <FAST/Testing.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/ImagePatch/ImageToBatchGenerator.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/VolumeRenderer/AlphaBlendingVolumeRenderer.hpp>

using namespace fast;

TEST_CASE("Patch generator for WSI", "[fast][wsi][PatchGenerator][visual]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512);
    generator->setPatchLevel(0);
    generator->setInputConnection(importer->getOutputPort());

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(generator->getOutputPort());
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(2000);
    window->set2DMode();
    window->start();
}

TEST_CASE("Patch generator for volumes", "[fast][volume][PatchGenerator][visual]") {
    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CT/CT-Thorax.mhd");

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512, 32);
    generator->setInputConnection(importer->getOutputPort());

    auto renderer = AlphaBlendingVolumeRenderer::New();
    renderer->setTransferFunction(TransferFunction::CT_Blood_And_Bone());
    renderer->addInputConnection(generator->getOutputPort());
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(3000);
    window->start();
}

TEST_CASE("Patch generator and stitcher for volumes", "[fast][volume][patchgenerator][PatchStitcher][visual]") {
    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CT/CT-Thorax.mhd");

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512, 32);
    generator->setInputConnection(importer->getOutputPort());

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(generator->getOutputPort());

    auto renderer = AlphaBlendingVolumeRenderer::New();
    renderer->setTransferFunction(TransferFunction::CT_Blood_And_Bone());
    renderer->addInputConnection(stitcher->getOutputPort());
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(1000);
    window->start();
}

TEST_CASE("Patch generator and stitcher for WSI", "[fast][wsi][PatchStitcher][visual]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");
    auto pyramid = importer->runAndGetOutputData<ImagePyramid>();
    auto level2Image = pyramid->getAccess(ACCESS_READ)->getLevelAsImage(2);

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512);
    generator->setPatchLevel(2);
    generator->setOverlap(0.1);
    generator->setInputData(pyramid);

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(generator->getOutputPort());

    auto renderer = ImageRenderer::create(-1,-1, 0.5);
    renderer->addInputConnection(stitcher->getOutputPort());
    auto renderer2 = ImageRenderer::create(-1,-1, 0.5);
    renderer2->addInputData(level2Image);
    auto window = SimpleWindow::New();
    window->addRenderer(renderer2);
    window->addRenderer(renderer);
    window->getView()->setBackgroundColor(Color::Black());
    window->setTimeout(3000);
    window->set2DMode();
    window->start();
}

TEST_CASE("Patch generator, sticher and image to batch generator for WSI", "[fast][wsi][ImageToBatchGenerator]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/A05.svs");

    auto generator = PatchGenerator::create(512, 512, 1, 2)
            ->connect(importer);

    auto batchGenerator = ImageToBatchGenerator::create(4)
            ->connect(generator);

    Batch::pointer batch;
    auto stream = DataStream(batchGenerator);
    while(!stream.isDone()) {
        batch = stream.getNextFrame<Batch>();
        std::cout << "Got a batch" << std::endl;
    }
    std::cout << "Done" << std::endl;
}