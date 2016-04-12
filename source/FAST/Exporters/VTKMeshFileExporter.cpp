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

    Mesh::pointer mesh = getStaticInputData<Mesh>();

    char dimensions = mesh->getDimensions();

    // Get transformation
    AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(mesh);

    std::ofstream file(mFilename.c_str());

    if(!file.is_open())
        throw Exception("Unable to open the file " + mFilename);

    // Write header
    file << "# vtk DataFile Version 3.0\n"
            "vtk output\n"
            "ASCII\n"
            "DATASET POLYDATA\n";

    // Write vertices
    MeshAccess::pointer access = mesh->getMeshAccess(ACCESS_READ);
    std::vector<MeshVertex> vertices = access->getVertices();
    file << "POINTS " << vertices.size() << " float\n";
    for(int i = 0; i < vertices.size(); i++) {
        MeshVertex vertex = vertices[i];
        if(dimensions == 3) {
			vertex.getPosition() = (transform->matrix()*vertex.getPosition().homogeneous()).head(3);
			file << vertex.getPosition().x() << " " << vertex.getPosition().y() << " " << vertex.getPosition().z() << "\n";
        } else {
			file << vertex.getPosition().x() << " " << vertex.getPosition().y() << " " << 0 << "\n";
        }
    }

    if(dimensions == 3) {
		// Write triangles
		std::vector<VectorXui> triangles = access->getTriangles();
		file << "POLYGONS " << mesh->getNrOfTriangles() << " " << mesh->getNrOfTriangles()*4 << "\n";
		for(int i = 0; i < triangles.size(); i++) {
			Vector3ui triangle = triangles[i];
			file << "3 " << triangle.x() << " " << triangle.y() << " " << triangle.z() << "\n";
		}
    } else {
    	// Write lines
		std::vector<VectorXui> lines = access->getLines();
		file << "LINES " << mesh->getNrOfLines() << " " << mesh->getNrOfLines()*3 << "\n";
		for(int i = 0; i < lines.size(); i++) {
			Vector2ui line = lines[i];
			file << "2 " << line.x() << " " << line.y() << "\n";
		}
    }

    // Write.getNormal()s
    file << "POINT_DATA " << vertices.size() << "\n";
    file << "NORMALS Normals float\n";
    for(int i = 0; i < vertices.size(); i++) {
        MeshVertex vertex = vertices[i];
        VectorXf normal = vertex.getNormal();

        if(dimensions == 3)
			normal = transform->linear()*normal; // Transform the normal

        // Normalize it
        float length = normal.norm();
        if(length == 0) { // prevent NaN situations
			file << "0 1 0\n";
        } else {
        	normal.normalize();
			if(dimensions == 3) {
				file << normal.x() << " " << normal.y() << " " << normal.z() << "\n";
			} else {
				file << normal.x() << " " << normal.y() << " 0" << "\n";
			}
        }
    }

    file.close();
}

}
