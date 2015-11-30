#include "FAST/Algorithms/IterativeClosestPoint/IterativeClosestPoint.hpp"
#include "FAST/Importers/VTKPointSetFileImporter.hpp"
#include "FAST/Visualization/PointRenderer/PointRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main() {
    // Import two point sets A and B
    VTKPointSetFileImporter::pointer importerA = VTKPointSetFileImporter::New();
    importerA->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");

    VTKPointSetFileImporter::pointer importerB = VTKPointSetFileImporter::New();
    importerB->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");

    // Apply a transformation to point set B
    Vector3f translation(0.01, 0, 0.01);
    Vector3f rotation(0.5, 0, 0);
    AffineTransformation::pointer transformation = AffineTransformation::New();
    transformation->translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transformation->rotate(R);
    importerB->update();
    importerB->getStaticOutputData<PointSet>()->getSceneGraphNode()->setTransformation(transformation);

    // Perform the registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingPointSetPort(importerA->getOutputPort());
    icp->setFixedPointSetPort(importerB->getOutputPort());
    icp->update();

    // Apply transformation to A
    importerA->getStaticOutputData<PointSet>()->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());

    Reporter::info() << "Registration result: " << Reporter::end;
    Reporter::info() << "Rotation: " << icp->getOutputTransformation()->getEulerAngles().transpose() << Reporter::end;
    Reporter::info() << "Translation:" << icp->getOutputTransformation()->translation().transpose() << Reporter::end;

    // Visualize the two point sets
    PointRenderer::pointer renderer = PointRenderer::New();
    renderer->addInputConnection(importerA->getOutputPort(), Color::Blue(), 10);
    renderer->addInputConnection(importerB->getOutputPort(), Color::Green(), 5);
    renderer->setDefaultDrawOnTop(true);

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(5*1000);
    window->start();
}
