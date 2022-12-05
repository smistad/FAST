#include "FAST/Testing.hpp"
#include "FAST/Algorithms/IterativeClosestPoint/IterativeClosestPoint.hpp"
#include "FAST/Importers/VTKMeshFileImporter.hpp"

namespace fast {

TEST_CASE("IterativeClosestPoint", "[fast][IterativeClosestPoint][icp]") {

    std::vector<MeshVertex> vertices;
    vertices.push_back(MeshVertex(Vector3f(2,2,1)));
    vertices.push_back(MeshVertex(Vector3f(6,2,2)));
    vertices.push_back(MeshVertex(Vector3f(2,6,1)));
    auto A = Mesh::create(vertices);

    std::vector<MeshVertex> verticesB;
    verticesB.push_back(MeshVertex(Vector3f(3,2,0)));
    verticesB.push_back(MeshVertex(Vector3f(7,0,0)));
    verticesB.push_back(MeshVertex(Vector3f(9,5,0)));
    verticesB.push_back(MeshVertex(Vector3f(2,1,1)));
    verticesB.push_back(MeshVertex(Vector3f(2,1,8)));
    auto B = Mesh::create(verticesB);

    auto icp = IterativeClosestPoint::create()
            ->connectFixed(A)
            ->connectMoving(B);

    CHECK_NOTHROW(icp->run());
}

TEST_CASE("ICP on two point sets translation only", "[fast][IterativeClosestPoint][icp]") {

    Vector3f translation(0.01, 0.0, 0.01);

    VTKMeshFileImporter::pointer importerA = VTKMeshFileImporter::New();
    importerA->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    auto importerAPort = importerA->getOutputPort();
    VTKMeshFileImporter::pointer importerB = VTKMeshFileImporter::New();
    importerB->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    auto importerBPort = importerB->getOutputPort();

    // Apply a transformation to B surface
    Affine3f transform = Affine3f::Identity();
    transform.translate(translation);
    importerB->update();
    Mesh::pointer B = importerBPort->getNextFrame<Mesh>();
    B->getSceneGraphNode()->setTransform(transform);

    importerA->update();
    Mesh::pointer A = importerAPort->getNextFrame<Mesh>();

    // Do ICP registration
    auto icp = IterativeClosestPoint::create(IterativeClosestPoint::TRANSLATION)->connectFixed(B)->connectMoving(A);
    icp->run();

    // Validate result
    A->getSceneGraphNode()->setTransform(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation()->get().rotation().eulerAngles(0, 1, 2);
    Vector3f detectedTranslation = icp->getOutputTransformation()->get().translation();

    CHECK(detectedTranslation.x() == Approx(translation.x()));
    CHECK(detectedTranslation.y() == Approx(translation.y()).scale(1.0));
    CHECK(detectedTranslation.z() == Approx(translation.z()));
    CHECK(detectedRotation.x() == Approx(0).scale(1.0));
    CHECK(detectedRotation.y() == Approx(0).scale(1.0));
    CHECK(detectedRotation.z() == Approx(0).scale(1.0));
}

TEST_CASE("ICP on two point sets", "[fast][IterativeClosestPoint][icp]") {

    Vector3f translation(0.01, 0, 0.01);
    Vector3f rotation(0.5, 0, 0);

    VTKMeshFileImporter::pointer importerA = VTKMeshFileImporter::New();
    importerA->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    auto importerAPort = importerA->getOutputPort();
    importerA->update();
    Mesh::pointer A = importerAPort->getNextFrame<Mesh>();
    VTKMeshFileImporter::pointer importerB = VTKMeshFileImporter::New();
    importerB->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    auto importerBPort = importerB->getOutputPort();
    importerB->update();
    Mesh::pointer B = importerBPort->getNextFrame<Mesh>();

    // Apply a transformation to B surface
    Affine3f transform = Affine3f::Identity();
    transform.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transform.rotate(R);
    B->getSceneGraphNode()->setTransform(transform);

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingMesh(A);
    icp->setFixedMesh(B);
    icp->update();

    // Validate result
    A->getSceneGraphNode()->setTransform(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation()->get().rotation().eulerAngles(0, 1, 2);
    Vector3f detectedTranslation = icp->getOutputTransformation()->get().translation();

    CHECK(detectedTranslation.x() == Approx(translation.x()));
    CHECK(detectedTranslation.y() == Approx(translation.y()).scale(1.0));
    CHECK(detectedTranslation.z() == Approx(translation.z()));
    CHECK(detectedRotation.x() == Approx(rotation.x()));
    CHECK(detectedRotation.y() == Approx(rotation.y()));
    CHECK(detectedRotation.z() == Approx(rotation.z()).scale(1.0));
}

TEST_CASE("ICP on two point sets which are already transformed by scene graph", "[fast][IterativeClosestPoint][icp]") {

    Vector3f translation(0.01, 0, 0.01);
    Vector3f rotation(0.5, 0, 0);

    VTKMeshFileImporter::pointer importerA = VTKMeshFileImporter::New();
    importerA->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    auto importerAPort = importerA->getOutputPort();
    VTKMeshFileImporter::pointer importerB = VTKMeshFileImporter::New();
    importerB->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    auto importerBPort = importerB->getOutputPort();

    importerA->update();
    importerB->update();
    Mesh::pointer A = importerAPort->getNextFrame<Mesh>();
    Mesh::pointer B = importerBPort->getNextFrame<Mesh>();

    auto FASTtransformInit = Transform::create();
    SceneGraph::insertParentNodeToData(A, FASTtransformInit);
    SceneGraph::insertParentNodeToData(B, FASTtransformInit);

    // Apply a transformation to B surface
    Affine3f transform = Affine3f::Identity();
    transform.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transform.rotate(R);
    B->getSceneGraphNode()->setTransform(transform);

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingMesh(A);
    icp->setFixedMesh(B);
    icp->update();

    // Validate result
    A->getSceneGraphNode()->setTransform(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation()->get().rotation().eulerAngles(0, 1, 2);
    Vector3f detectedTranslation = icp->getOutputTransformation()->get().translation();

    CHECK(detectedTranslation.x() == Approx(translation.x()));
    CHECK(detectedTranslation.y() == Approx(translation.y()).scale(1.0));
    CHECK(detectedTranslation.z() == Approx(translation.z()));
    CHECK(detectedRotation.x() == Approx(rotation.x()));
    CHECK(detectedRotation.y() == Approx(rotation.y()));
    CHECK(detectedRotation.z() == Approx(rotation.z()).scale(1.0));
}



} // end namespace fast
