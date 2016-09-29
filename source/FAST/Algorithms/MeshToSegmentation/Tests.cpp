#include "MeshToSegmentation.hpp"
#include "FAST/Testing.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/DualViewWindow.hpp"

using namespace fast;

TEST_CASE("MeshToSegmentation 2D", "[fast][MeshToSegmentation]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US/CarotidArtery/Right/US-2D_0.mhd");

    Mesh::pointer mesh = Mesh::New();
    std::vector<Vector2f> vertices = {
            Vector2f(1, 1),
            Vector2f(1, 25),
            Vector2f(25, 20),
            Vector2f(20, 1)
    };
    std::vector<Vector2f> normals = {
            Vector2f(10, 10),
            Vector2f(10, 100),
            Vector2f(100, 100),
            Vector2f(100, 10)
    };
    std::vector<VectorXui> connections = {
            Vector2ui(0, 1),
            Vector2ui(1, 2),
            Vector2ui(2, 3),
            Vector2ui(3, 0)
    };

    mesh->create(vertices, normals, connections);

    MeshToSegmentation::pointer meshToSeg = MeshToSegmentation::New();
    meshToSeg->setInputData(0, mesh);
    meshToSeg->setInputConnection(1, importer->getOutputPort());

    ImageRenderer::pointer segRenderer = ImageRenderer::New();
    segRenderer->addInputConnection(meshToSeg->getOutputPort());
    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    DualViewWindow::pointer window = DualViewWindow::New();
    window->setWidth(1024);
    window->addRendererToBottomRightView(segRenderer);
    window->addRendererToTopLeftView(imageRenderer);
    window->getBottomRightView()->set2DMode();
    window->getTopLeftView()->set2DMode();
    window->start();
}