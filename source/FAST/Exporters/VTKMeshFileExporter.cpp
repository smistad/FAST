#include "VTKMeshFileExporter.hpp"
#include "FAST/Data/Mesh.hpp"
#include <fstream>
#include "FAST/SceneGraph.hpp"

namespace fast {

void VTKMeshFileExporter::setFilename(std::string filename) {
    mFilename = filename;
}

VTKMeshFileExporter::VTKMeshFileExporter() {
    createInputPort<Mesh>(0);
    mFilename = "";
}

void VTKMeshFileExporter::execute() {
    if(mFilename == "")
        throw Exception("No filename given to the VTKMeshFileExporter");

    Mesh::pointer surface = getStaticInputData<Mesh>();

    // Get transformation
    AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(surface);

    std::ofstream file(mFilename.c_str());

    if(!file.is_open())
        throw Exception("Unable to open the file " + mFilename);

    // Write header
    file << "# vtk DataFile Version 3.0\n"
            "vtk output\n"
            "ASCII\n"
            "DATASET POLYDATA\n";

    // Write vertices
    MeshAccess::pointer access = surface->getMeshAccess(ACCESS_READ);
    std::vector<MeshVertex> vertices = access->getVertices();
    file << "POINTS " << vertices.size() << " float\n";
    for(int i = 0; i < vertices.size(); i++) {
        MeshVertex vertex = vertices[i];
        vertex.getPosition() = (transform->matrix()*vertex.getPosition().homogeneous()).head(3);
        file << vertex.getPosition().x() << " " << vertex.getPosition().y() << " " << vertex.getPosition().z() << "\n";
    }

    // Write triangles
    std::vector<VectorXui> triangles = access->getTriangles();
    file << "POLYGONS " << surface->getNrOfTriangles() << " " << surface->getNrOfTriangles()*4 << "\n";
    for(int i = 0; i < triangles.size(); i++) {
        Vector3ui triangle = triangles[i];
        file << "3 " << triangle.x() << " " << triangle.y() << " " << triangle.z() << "\n";
    }

    // Write.getNormal()s
    file << "POINT_DATA " << vertices.size() << "\n";
    file << "NORMALS Normals float\n";
    for(int i = 0; i < vertices.size(); i++) {
        MeshVertex vertex = vertices[i];
        // Transform it
        vertex.getNormal() = transform->linear()*vertex.getNormal();
        // Normalize it
        float length = vertex.getNormal().norm();
        if(length == 0) { // prevent NaN situations
            file << "0 1 0\n";
        } else {
            file << (vertex.getNormal().x()/length) << " " << (vertex.getNormal().y()/length) << " " << (vertex.getNormal().z()/length) << "\n";
        }
    }

    file.close();
}

}
