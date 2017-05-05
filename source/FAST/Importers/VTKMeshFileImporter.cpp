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
    mFunctions["POINTS"] = std::bind(&VTKMeshFileImporter::processPoints, this, std::placeholders::_1, std::placeholders::_2);
    mFunctions["NORMALS"] = std::bind(&VTKMeshFileImporter::processNormals, this, std::placeholders::_1, std::placeholders::_2);
    mFunctions["LINES"] = std::bind(&VTKMeshFileImporter::processLines, this, std::placeholders::_1, std::placeholders::_2);
    mFunctions["POLYGON"] = std::bind(&VTKMeshFileImporter::processTriangles, this, std::placeholders::_1, std::placeholders::_2);
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

void VTKMeshFileImporter::processLines(std::ifstream& file, std::string line) {
    std::cout << "processing normals" << std::endl;
}

void VTKMeshFileImporter::processTriangles(std::ifstream& file, std::string line) {
    std::cout << "processing triangles" << std::endl;
}

void VTKMeshFileImporter::processNormals(std::ifstream& file, std::string line) {
    std::cout << "processing normals" << std::endl;
    std::vector<std::string> tokens = split(line);
    //if(std::stoi(tokens.at(1)) != mVertices.size()) {
    //    throw Exception("Number of normals must be equal to number of points", __LINE__, __FILE__);
    //}
    uint counter = 0;
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
            mVertices.at(counter).setNormal(v);
            ++counter;
        }
    }
}

void VTKMeshFileImporter::processPoints(std::ifstream& file, std::string line) {
    std::cout << "Processing points" << std::endl;
    while(getline(file, line)) {
        trim(line);
        if(line.size() == 0)
            break;

        if(!(isdigit(line[0]) || line[0] == '-')) {
            // Has reached end
            break;
        }

        std::cout << line << std::endl;
        std::vector<std::string> tokens = split(line);

        for(int i = 0; i < tokens.size(); i += 3) {
            Vector3f v;
            v(0) = std::stof(tokens[i]);
            v(1) = std::stof(tokens[i+1]);
            v(2) = std::stof(tokens[i+2]);
            mVertices.push_back(MeshVertex(v));
        }
    }
}

void VTKMeshFileImporter::execute() {
    getReporter().setReportMethod(Reporter::COUT);
    if(mFilename == "")
        throw Exception("No filename given to the VTKMeshFileImporter");

    // Try to open file and check that it exists
    std::ifstream file(mFilename.c_str());
    std::string line;
    if(!file.is_open()) {
        throw FileNotFoundException(mFilename);
    }
    Mesh::pointer output = getOutputData<Mesh>(0);

    getline(file, line);
    while(!file.eof()) {
        trim(line);
        if(line.size() == 0 || line[0] == '#') {
            // Skip empty lines and comments
            getline(file, line);
            continue;
        }
        std::cout << "New line: " << line << std::endl;
        std::string key = line.substr(0, line.find(" "));
        if(mFunctions.count(key) > 0) {
            mFunctions[key](file, line);
        } else {
            // Line not recognized, ignore..
        }
        getline(file, line);
    }

    if(mVertices.size() == 0) {
        throw Exception("No points found in file " + mFilename);
    }
    std::vector<VectorXui> lines;
    std::vector<VectorXui> triangles;
    std::vector<Vector3f> normals;

    /*
    // Read vertices
    std::vector<Vector3f> vertices;
    if(!gotoLineWithString(file, "POINTS")) {
        throw Exception("Found no vertices in the VTK file");
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
    // set stream to start
    file.clear();
    file.seekg(0, std::ios::beg);

    // Read lines
    std::vector<VectorXui> lines;
    if(gotoLineWithString(file, "LINES")) {
        std::cout << "Found lines.." << std::endl;
        while(getline(file, line)) {
            trim(line);
            if(line.size() == 0)
                break;

            if(!(isdigit(line[0]) || line[0] == '-')) {
                // Has reached end
                break;
            }

            std::vector<std::string> tokens = split(line);

            if(tokens.size() != 3) {
                throw Exception("Error while reading lines in VTKMeshFileImporter. Check format.");
            }

            Vector2ui line;
            line(0) = std::stoi(tokens[1]);
            line(1) = std::stoi(tokens[2]);

            lines.push_back(line);
        }
    }
    // Set stream to start
    file.clear();
    file.seekg(0, std::ios::beg);

    // Read triangles (other types of polygons not supported yet)
    std::vector<VectorXui> triangles;
    if(gotoLineWithString(file, "POLYGONS")) {
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
                throw Exception(
                        "The VTKMeshFileImporter currently only supports reading files with triangles. Encountered a non-triangle. Aborting.");
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
    }
    // Set stream to start
    file.clear();
    file.seekg(0, std::ios::beg);

    // Check if there are colors for the vertices
    if(gotoLineWithString(file, "VECTORS")) {

    }
     */

    // Add data to output
    if(lines.size() > 0) {
        std::cout << "adding lines.." << std::endl;
        //output->create(mVertices, normals, lines);
        reportInfo() << "MESH IMPORTED vertices " << mVertices.size() << " normals " << normals.size() << " lines " << lines.size() << Reporter::end;
    } else if(triangles.size() > 0) {
        //output->create(mVertices, normals, triangles);
        reportInfo() << "MESH IMPORTED vertices " << mVertices.size() << " normals " << normals.size() << " triangles " << triangles.size() << Reporter::end;
    } else {
        output->create(mVertices);
        reportInfo() << "MESH IMPORTED vertices " << mVertices.size() << reportEnd();
    }
}

} // end namespace fast


