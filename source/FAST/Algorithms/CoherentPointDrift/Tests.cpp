#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Importers/VTKMeshFileImporter.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include "FAST/Testing.hpp"
#include "FAST/Visualization/VertexRenderer/VertexRenderer.hpp"
#include "CoherentPointDrift.hpp"

using namespace fast;

Mesh::pointer getPointCloud() {
    auto importer = VTKMeshFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    auto port = importer->getOutputPort();
    importer->update(0);
    return port->getNextFrame<Mesh>();
}

TEST_CASE("cpd", "[fast][coherentpointdrift][visual][cpd]") {

    auto cloud1 = getPointCloud();
    auto cloud2 = getPointCloud();

    auto transform = AffineTransformation::New();
    Affine3f affine = Affine3f::Identity();
    affine.translate(Vector3f(0.10, 0, 0));
    transform->setTransform(affine);
    cloud2->getSceneGraphNode()->setTransformation(transform);

    auto cpd = CoherentPointDrift::New();
    cpd->setFixedMesh(cloud1);
    cpd->setMovingMesh(cloud2);

    auto renderer = VertexRenderer::New();
    renderer->addInputData(cloud1);
    renderer->addInputConnection(cpd->getOutputPort(), Color::Red(), 2.0);

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    //window->setTimeout(1000);
    window->start();

}
