/**
 * @example iterativeClosestPoint.cpp
 *
 * Iterative closest point (ICP) registration of two point sets A and B.
 */
#include "FAST/Algorithms/IterativeClosestPoint/IterativeClosestPoint.hpp"
#include <FAST/Importers/VTKMeshFileImporter.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include "FAST/Visualization/VertexRenderer/VertexRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"


using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Iterative closest point example (ICP)");
    parser.parse(argc, argv);

    // Import two point sets A and B
    auto importerA = VTKMeshFileImporter::create(Config::getTestDataPath() + "Surface_LV.vtk");
    auto A = importerA->runAndGetOutputData<Mesh>();

    auto importerB = VTKMeshFileImporter::create(Config::getTestDataPath() + "Surface_LV.vtk");
    auto B = importerB->runAndGetOutputData<Mesh>();

    // Apply a transformation to point set B
    Vector3f translation(0.01, 0, 0.01);
    Vector3f rotation(0.5, 0, 0);
    Affine3f transform = Affine3f::Identity();
    transform.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transform.rotate(R);
    B->getSceneGraphNode()->setTransform(transform);

    // Perform the registration
    auto icp = IterativeClosestPoint::create()
            ->connectMoving(A)
            ->connectFixed(B);
    icp->run();

    // Apply transformation to A
    A->getSceneGraphNode()->setTransform(icp->getOutputTransformation());

    std::cout << "Registration result: " << std::endl;
    std::cout << "Rotation: " << icp->getOutputTransformation()->get().rotation().eulerAngles(0,1,2).transpose() << std::endl;
    std::cout << "Translation:" << icp->getOutputTransformation()->get().translation().transpose() << std::endl;

    // Visualize the two point sets
    auto renderer = VertexRenderer::create(10, Color::Blue(), true)
            ->connect(importerA);
    auto renderer2 = VertexRenderer::create(5, Color::Green(), true)
            ->connect(importerB);

    auto window = SimpleWindow3D::create()
            ->connect({renderer, renderer2});
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->run();
}
