#include "FAST/Utility.hpp"
#include <fstream>
#include "VTKMeshFileImporter.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

void VTKMeshFileImporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

VTKMeshFileImporter::VTKMeshFileImporter() {
    mFilename = "";
    mIsModified = true;
    createOutputPort<Mesh>(0, OUTPUT_STATIC);
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
        trim(line);
        if(line.size() == 0)
            break;

        if(!(isdigit(line[0]) || line[0] == '-')) {
            // Has reached end
            break;
        }

        std::vector<std::string> tokens = split(line);

        for(int i = 0; i < tokens.size(); i += 3) {
            Vector3f v;
            v(0) = std::stof(tokens[i]);
            v(1) = std::stof(tokens[i+1]);
            v(2) = std::stof(tokens[i+2]);
            vertices.push_back(v);
        }
    }
    file.seekg(0); // set stream to start

    // Read triangles (other types of polygons not supported yet)
    std::vector<VectorXui> triangles;
    if(!gotoLineWithString(file, "POLYGONS")) {
        throw Exception("Found no triangles in the VTK surface file");
    }
    while(getline(file, line)) {
        trim(line);
        if(line.size() == 0)
            break;

        if(!isdigit(line[0])) {
            // Has reached end
            break;
        }


        std::vector<std::string> tokens = split(line);

        if(std::stoi(tokens[0]) != 3) {
            throw Exception("The VTKMeshFileImporter currently only supports reading files with triangles. Encountered a non-triangle. Aborting.");
        }
        if(tokens.size() != 4) {
            throw Exception("Error while reading triangles in VTKMeshFileImporter. Check format.");
        }

        Vector3ui triangle;
        triangle(0) = std::stoi(tokens[1]);
        triangle(1) = std::stoi(tokens[2]);
        triangle(2) = std::stoi(tokens[3]);

        triangles.push_back(triangle);

    }
    file.seekg(0); // set stream to start

    // Read normals (if any)
    std::vector<Vector3f> normals;
    if(!gotoLineWithString(file, "NORMALS")) {
        // Create dummy normals
        for(uint i = 0; i < vertices.size(); i++) {
            Vector3f dummyNormal;
            normals.push_back(dummyNormal);
        }
    } else {
        while(getline(file, line)) {
            trim(line);
            if(line.size() == 0)
                break;

            if(!(isdigit(line[0]) || line[0] == '-')) {
                // Has reached end
                break;
            }

            std::vector<std::string> tokens = split(line);

            for(int i = 0; i < tokens.size(); i += 3) {
                Vector3f v;
                v(0) = std::stof(tokens[i]);
                v(1) = std::stof(tokens[i+1]);
                v(2) = std::stof(tokens[i+2]);
                normals.push_back(v);
            }
        }

        if(normals.size() != vertices.size()) {
            std::string message = "Read different amount of vertices (" + std::to_string(vertices.size()) + ") and normals (" + std::to_string(normals.size()) + ").";
            throw Exception(message);
        }
    }

    Mesh::pointer output = getOutputData<Mesh>(0);

    // Add data to output
    output->create(vertices, normals, triangles);
    reportInfo() << "MESH IMPORTED vertices " << vertices.size() << " normals " << normals.size() << " triangles " << triangles.size() << Reporter::end;
}

} // end namespace fast


