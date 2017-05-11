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
    mFunctions["POLYGONS"] = std::bind(&VTKMeshFileImporter::processTriangles, this, std::placeholders::_1, std::placeholders::_2);
    mFunctions["VECTORS"] = std::bind(&VTKMeshFileImporter::processVectors, this, std::placeholders::_1, std::placeholders::_2);
}

void VTKMeshFileImporter::processLines(std::ifstream& file, std::string& line) {
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

        MeshLine line(std::stoi(tokens[1]), std::stoi(tokens[2]));

        mLines.push_back(line);
    }
}

void VTKMeshFileImporter::processTriangles(std::ifstream& file, std::string& line) {
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

        MeshTriangle triangle(
                std::stoi(tokens[1]),
                std::stoi(tokens[2]),
                std::stoi(tokens[3])
        );

        mTriangles.push_back(triangle);
    }
}

void VTKMeshFileImporter::processNormals(std::ifstream& file, std::string& line) {
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

void VTKMeshFileImporter::processVectors(std::ifstream& file, std::string& line) {
    std::vector<std::string> tokens = split(line);
    std::string name = tokens[1];
    if(name != "vertex_colors") {
        reportWarning() << "Unknown VECTORS data with name " << name << " in file " << mFilename << reportEnd();
    }
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

        if(name == "vertex_colors") {
            std::vector<std::string> tokens = split(line);
            for(int i = 0; i < tokens.size(); i += 3) {
                float red = std::stof(tokens[i]);
                float green = std::stof(tokens[i + 1]);
                float blue = std::stof(tokens[i + 2]);
                mVertices.at(counter).setColor(Color(red, green, blue));
                ++counter;
            }
        }
    }
}

void VTKMeshFileImporter::processPoints(std::ifstream& file, std::string& line) {
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
        std::string key = line.substr(0, line.find(" "));
        if(mFunctions.count(key) > 0) {
            mFunctions[key](file, line);
        } else {
            // Line not recognized, ignore..
            getline(file, line);
        }
    }

    if(mVertices.size() == 0) {
        throw Exception("No points found in file " + mFilename);
    }

    output->create(mVertices, mLines, mTriangles);
    reportInfo() << "MESH IMPORTED: vertices " << mVertices.size() << " lines " << mLines.size() << " triangles " << mTriangles.size() << Reporter::end();
}

} // end namespace fast


