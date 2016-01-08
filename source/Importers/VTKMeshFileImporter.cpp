#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include "VTKMeshFileImporter.hpp"

namespace fast {

void VTKMeshFileImporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

VTKMeshFileImporter::VTKMeshFileImporter() {
    mFilename = "";
    mIsModified = true;
}

inline bool gotoLineWithString(std::ifstream &file, std::string searchFor) {
    bool found = false;
    std::string line;
    while(getline(file, line)) {
        if(line.find(searchFor) != std::string::npos) {
            found = true;
            break;
        }
    }

    return found;
}

void VTKMeshFileImporter::execute() {
    if(mFilename == "")
        throw Exception("No filename given to the VTKMeshFileImporter");

    // Try to open file and check that it exists
    std::ifstream file(mFilename.c_str());
    std::string line;
    if(!file.is_open()) {
        throw FileNotFoundException(mFilename);
    }

    // Check file header?

    // Read vertices
    std::vector<Vector3f> vertices;
    if(!gotoLineWithString(file, "POINTS")) {
        throw Exception("Found no vertices in the VTK surface file");
    }
    while(getline(file, line)) {
        boost::trim(line);
        if(line.size() == 0)
            break;

        if(!(isdigit(line[0]) || line[0] == '-')) {
            // Has reached end
            break;
        }

        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        for(int i = 0; i < tokens.size(); i += 3) {
            Vector3f v;
            v(0) = boost::lexical_cast<float>(tokens[i]);
            v(1) = boost::lexical_cast<float>(tokens[i+1]);
            v(2) = boost::lexical_cast<float>(tokens[i+2]);
            vertices.push_back(v);
        }
    }
    file.seekg(0); // set stream to start

    // Read triangles (other types of polygons not supported yet)
    std::vector<Vector3ui> triangles;

	std::vector<Vector3f> normalsPerTriangle;

    if(!gotoLineWithString(file, "POLYGONS")) {
        throw Exception("Found no triangles in the VTK surface file");
    }
    while(getline(file, line)) {
        boost::trim(line);
        if(line.size() == 0)
            break;

        if(!isdigit(line[0])) {
            // Has reached end
            break;
        }


        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        if(boost::lexical_cast<int>(tokens[0]) != 3) {
            throw Exception("The VTKMeshFileImporter currently only supports reading files with triangles. Encountered a non-triangle. Aborting.");
        }
        if(tokens.size() != 4) {
            throw Exception("Error while reading triangles in VTKMeshFileImporter. Check format.");
        }

        Vector3ui triangle;
        triangle(0) = boost::lexical_cast<uint>(tokens[1]);
        triangle(1) = boost::lexical_cast<uint>(tokens[2]);
        triangle(2) = boost::lexical_cast<uint>(tokens[3]);
        triangles.push_back(triangle);

		//if (!gotoLineWithString(file, "NORMALS")) {
			
			//Cereate Surface Normal For Each Triangle
			Vector3f U(vertices[triangle(1)] - vertices[triangle(0)]);
			Vector3f V(vertices[triangle(2)] - vertices[triangle(0)]);

			Vector3f normalPerTriangle;
			normalPerTriangle(0) = ((U.y()*V.z()) - (U.z()*V.y()));
			normalPerTriangle(1) = ((U.z()*V.x()) - (U.x()*V.z()));
			normalPerTriangle(2) = ((U.x()*V.y()) - (U.y()*V.x()));

			normalsPerTriangle.push_back(normalPerTriangle);
		//}

    }
    file.seekg(0); // set stream to start

    // Read normals (if any)
    std::vector<Vector3f> normals; // normals per vertex
	

    if(!gotoLineWithString(file, "NORMALS")) {
        // Generate normals for each vertex
        for(uint i = 0; i < vertices.size(); i++) 
		{
			Vector3f normal(0.0, 0.0, 0.0);
			for (uint j = 0; j < triangles.size(); j++) 
				if (triangles[j].x() == i || triangles[j].y() == i || triangles[j].z() == i)
					normal += normalsPerTriangle[j];

			normal.normalize();
			normals.push_back(normal);
		}

    } else {
        while(getline(file, line)) {
            boost::trim(line);
            if(line.size() == 0)
                break;

            if(!(isdigit(line[0]) || line[0] == '-')) {
                // Has reached end
                break;
            }

            std::vector<std::string> tokens;
            boost::split(tokens, line, boost::is_any_of(" "));

            for(int i = 0; i < tokens.size(); i += 3) {
                Vector3f v;
                v(0) = boost::lexical_cast<float>(tokens[i]);
                v(1) = boost::lexical_cast<float>(tokens[i+1]);
                v(2) = boost::lexical_cast<float>(tokens[i+2]);
                normals.push_back(v);
            }
        }

        if(normals.size() != vertices.size()) {
            std::string message = "Read different amount of vertices (" + boost::lexical_cast<std::string>(vertices.size()) + ") and normals (" + boost::lexical_cast<std::string>(normals.size()) + ").";
            throw Exception(message);
        }
    }

    Mesh::pointer output = getOutputData<Mesh>(0);

    // Add data to output
    output->create(vertices, normals, triangles);
    std::cout << "MESH IMPORTED" << std::endl;
	std::cout << "vertices:" << vertices.size() << std::endl;
	std::cout << "normals:" << normals.size() << std::endl;
	std::cout << "triangles:" << triangles.size() << std::endl;

}

} // end namespace fast


