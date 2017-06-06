#include "MeshToSegmentation.hpp"
#include "FAST/Testing.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/DualViewWindow.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"

using namespace fast;

TEST_CASE("MeshToSegmentation 2D", "[fast][MeshToSegmentation][2d][visual]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_0.mhd");

    Mesh::pointer mesh = Mesh::New();
    std::vector<MeshVertex> vertices = {
            MeshVertex(Vector3f(1, 1, 0)),
            MeshVertex(Vector3f(1, 25, 0)),
            MeshVertex(Vector3f(25, 20, 0)),
            MeshVertex(Vector3f(20, 1, 0)),
    };
    std::vector<MeshLine> lines = {
            MeshLine(0, 1),
            MeshLine(1, 2),
            MeshLine(2, 3),
            MeshLine(3, 0)
    };

    mesh->create(vertices, lines);

    MeshToSegmentation::pointer meshToSeg = MeshToSegmentation::New();
    meshToSeg->setInputData(0, mesh);
    meshToSeg->setInputConnection(1, importer->getOutputPort());
    //meshToSeg->setOutputImageResolution(50, 50);

    SegmentationRenderer::pointer segRenderer = SegmentationRenderer::New();
    segRenderer->addInputConnection(meshToSeg->getOutputPort());
    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    DualViewWindow::pointer window = DualViewWindow::New();
    window->setWidth(1024);
    window->addRendererToBottomRightView(segRenderer);
    window->addRendererToTopLeftView(imageRenderer);
    window->addRendererToTopLeftView(segRenderer);
    window->getBottomRightView()->set2DMode();
    window->getTopLeftView()->set2DMode();
    window->setTimeout(500);
    window->start();
}

TEST_CASE("MeshToSegmentation 3D", "[fast][MeshToSegmentation][3d][visual]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/Ball/US-3Dt_0.mhd");

    Mesh::pointer mesh = Mesh::New();
    std::vector<MeshVertex> vertices = {
            MeshVertex(Vector3f(1, 1, 1)),
            MeshVertex(Vector3f(1, 1, 10)),
            MeshVertex(Vector3f(1, 10, 10)),

            MeshVertex(Vector3f(1, 1, 1)),
            MeshVertex(Vector3f(1, 1, 10)),
            MeshVertex(Vector3f(30, 15, 15)),

            MeshVertex(Vector3f(1, 1, 10)),
            MeshVertex(Vector3f(1, 10, 10)),
            MeshVertex(Vector3f(30, 15, 15)),

            MeshVertex(Vector3f(1, 1, 1)),
            MeshVertex(Vector3f(1, 10, 10)),
            MeshVertex(Vector3f(30, 15, 15))
    };
    std::vector<MeshTriangle> triangles = {
            MeshTriangle(0, 1, 2),
            MeshTriangle(3, 4, 5),
            MeshTriangle(6, 7, 8),
            MeshTriangle(9, 10, 11)
    };

    mesh->create(vertices, {}, triangles);

    MeshToSegmentation::pointer meshToSeg = MeshToSegmentation::New();
    meshToSeg->setInputData(0, mesh);
    meshToSeg->setInputConnection(1, importer->getOutputPort());
    meshToSeg->setOutputImageResolution(40, 40, 40);

    SliceRenderer::pointer imageRenderer = SliceRenderer::New();
    imageRenderer->setInputConnection(importer->getOutputPort());

    SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
    extraction->setInputConnection(meshToSeg->getOutputPort());

    TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
    TriangleRenderer->setInputConnection(extraction->getOutputPort());

    DualViewWindow::pointer window = DualViewWindow::New();
    window->setWidth(1024);
    window->addRendererToBottomRightView(TriangleRenderer);
    window->addRendererToTopLeftView(imageRenderer);
    //window->getBottomRightView()->set2DMode();
    //window->getTopLeftView()->set2DMode();
    window->setTimeout(500);
    window->start();
}
