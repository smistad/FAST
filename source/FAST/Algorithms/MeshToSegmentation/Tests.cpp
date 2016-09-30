#include "MeshToSegmentation.hpp"
#include "FAST/Testing.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/DualViewWindow.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"

using namespace fast;

TEST_CASE("MeshToSegmentation 2D", "[fast][MeshToSegmentation][2d]") {
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

TEST_CASE("MeshToSegmentation 3D", "[fast][MeshToSegmentation][3d]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US/Ball/US-3Dt_0.mhd");

    Mesh::pointer mesh = Mesh::New();
    std::vector<Vector3f> vertices = {
            Vector3f(1, 1, 1),
            Vector3f(1, 1, 10),
            Vector3f(1, 10, 10),

            Vector3f(1, 1, 1),
            Vector3f(1, 1, 10),
            Vector3f(30, 15, 15),

            Vector3f(1, 1, 10),
            Vector3f(1, 10, 10),
            Vector3f(30, 15, 15),

            Vector3f(1, 1, 1),
            Vector3f(1, 10, 10),
            Vector3f(30, 15, 15)
    };
    std::vector<Vector3f> normals = {
            Vector3f(1, 1, 1),
            Vector3f(1, 1, 2),
            Vector3f(1, 2, 2),
            Vector3f(1, 1, 1),
            Vector3f(1, 1, 2),
            Vector3f(3, 1.5, 1.5),
            Vector3f(1, 1, 2),
            Vector3f(1, 2, 2),
            Vector3f(3, 1.5, 1.5),
            Vector3f(1, 1, 1),
            Vector3f(1, 2, 2),
            Vector3f(3, 1.5, 1.5)
    };
    std::vector<VectorXui> connections = {
            Vector3ui(0, 1, 2),
            Vector3ui(3, 4, 5),
            Vector3ui(6, 7, 8),
            Vector3ui(9, 10, 11)
    };

    mesh->create(vertices, normals, connections);

    MeshToSegmentation::pointer meshToSeg = MeshToSegmentation::New();
    meshToSeg->setInputData(0, mesh);
    meshToSeg->setInputConnection(1, importer->getOutputPort());

    SliceRenderer::pointer segRenderer = SliceRenderer::New();
    segRenderer->setInputConnection(meshToSeg->getOutputPort());
    segRenderer->setSliceToRender(50);
    SliceRenderer::pointer imageRenderer = SliceRenderer::New();
    imageRenderer->setInputConnection(importer->getOutputPort());

    SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
    extraction->setInputConnection(meshToSeg->getOutputPort());

    MeshRenderer::pointer meshRenderer = MeshRenderer::New();
    meshRenderer->setInputConnection(extraction->getOutputPort());

    DualViewWindow::pointer window = DualViewWindow::New();
    window->setWidth(1024);
    window->addRendererToBottomRightView(meshRenderer);
    window->addRendererToTopLeftView(imageRenderer);
    //window->getBottomRightView()->set2DMode();
    //window->getTopLeftView()->set2DMode();
    window->start();
}
