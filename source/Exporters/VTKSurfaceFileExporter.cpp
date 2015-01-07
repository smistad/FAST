#include "VTKSurfaceFileExporter.hpp"
#include <fstream>
#include "SceneGraph.hpp"

namespace fast {

void VTKSurfaceFileExporter::setInput(MeshData::pointer input) {
    setParent(input);
    mInput = input;
}

void VTKSurfaceFileExporter::setFilename(std::string filename) {
    mFilename = filename;
}

VTKSurfaceFileExporter::VTKSurfaceFileExporter() {
    mFilename = "";
}

void VTKSurfaceFileExporter::execute() {
    if(mFilename == "")
        throw Exception("No filename given to the VTKSurfaceFileExporter");

    if(!mInput.isValid())
        throw Exception("No valid input given to the VTKSurfaceFileExporter");

    Mesh::pointer surface;
    if(mInput->isDynamicData()) {
        surface = DynamicMesh::pointer(mInput)->getNextFrame(mPtr);
    } else {
        surface = mInput;
    }


    // Get transformation
    LinearTransformation transform = SceneGraph::getLinearTransformationFromData(surface);

    std::ofstream file(mFilename.c_str());

    if(!file.is_open())
        throw Exception("Unable to open the file " + mFilename);

    // Write header
    file << "# vtk DataFile Version 3.0\n"
            "vtk output\n"
            "ASCII\n"
            "DATASET POLYDATA\n";

    // Write vertices
    SurfacePointerAccess access = surface->getSurfacePointerAccess(ACCESS_READ);
    std::vector<SurfaceVertex> vertices = access.getVertices();
    file << "POINTS " << vertices.size() << " float\n";
    for(int i = 0; i < vertices.size(); i++) {
        SurfaceVertex vertex = vertices[i];
        vertex.position = transform*vertex.position;
        file << vertex.position.x() << " " << vertex.position.y() << " " << vertex.position.z() << "\n";
    }

    // Write triangles
    std::vector<Vector3ui> triangles = access.getTriangles();
    file << "POLYGONS " << surface->getNrOfTriangles() << " " << surface->getNrOfTriangles()*4 << "\n";
    for(int i = 0; i < triangles.size(); i++) {
        Vector3ui triangle = triangles[i];
        file << "3 " << triangle.x() << " " << triangle.y() << " " << triangle.z() << "\n";
    }

    // Write normals
    file << "POINT_DATA " << vertices.size() << "\n";
    file << "NORMALS Normals float\n";
    for(int i = 0; i < vertices.size(); i++) {
        SurfaceVertex vertex = vertices[i];
        // Transform it
        vertex.normal = transform.getTransform().linear()*vertex.normal;
        // Normalize it
        float length = vertex.normal.norm();
        if(length == 0) { // prevent NaN situations
            file << "0 1 0\n";
        } else {
            file << (vertex.normal.x()/length) << " " << (vertex.normal.y()/length) << " " << (vertex.normal.z()/length) << "\n";
        }
    }

    file.close();
}

}
