#include "VTKSurfaceFileExporter.hpp"
#include <fstream>
#include "SceneGraph.hpp"

namespace fast {

void VTKSurfaceFileExporter::setInput(SurfaceData::pointer input) {
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

    Surface::pointer surface;
    if(mInput->isDynamicData()) {
        surface = DynamicSurface::pointer(mInput)->getNextFrame();
    } else {
        surface = mInput;
    }


    // Get transformation
    SceneGraph& graph = SceneGraph::getInstance();
    LinearTransformation transform = graph.getLinearTransformationFromNode(graph.getDataNode(surface));

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
    std::vector<Uint3> triangles = access.getTriangles();
    file << "POLYGONS " << surface->getNrOfTriangles() << " " << surface->getNrOfTriangles()*4 << "\n";
    for(int i = 0; i < triangles.size(); i++) {
        Uint3 triangle = triangles[i];
        file << "3 " << triangle.x() << " " << triangle.y() << " " << triangle.z() << "\n";
    }

    // Remove the translation part of the linear transformation for the normals
    transform(0,3) = 0;
    transform(1,3) = 0;
    transform(2,3) = 0;

    // Write normals
    file << "POINT_DATA " << vertices.size() << "\n";
    file << "NORMALS Normals float\n";
    for(int i = 0; i < vertices.size(); i++) {
        SurfaceVertex vertex = vertices[i];
        // Transform it
        vertex.normal = transform*vertex.normal;
        // Normalize it
        float length = sqrt(vertex.normal.x()*vertex.normal.x()+vertex.normal.y()*vertex.normal.y()+vertex.normal.z()*vertex.normal.z());
        if(length == 0) { // prevent NaN situations
            file << "0 1 0\n";
        } else {
            file << (vertex.normal.x()/length) << " " << (vertex.normal.y()/length) << " " << (vertex.normal.z()/length) << "\n";
        }
    }

    file.close();
}

}
