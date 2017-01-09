#include "VTKPointSetFileImporter.hpp"
#include "FAST/Data/PointSet.hpp"
#include "FAST/Utility.hpp"
#include <fstream>

namespace fast {

void VTKPointSetFileImporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

VTKPointSetFileImporter::VTKPointSetFileImporter() {
    mFilename = "";
    mIsModified = true;
    createOutputPort<PointSet>(0, OUTPUT_STATIC);
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

void VTKPointSetFileImporter::execute() {
    if(mFilename == "")
        throw Exception("No filename given to the VTKPointSetFileImporter");

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

    // Add data to output
    PointSet::pointer output = getOutputData<PointSet>(0);
    output->create(vertices);
}

} // end namespace fast


