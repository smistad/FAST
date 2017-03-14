#include "FileExporter.hpp"

namespace fast {

void FileExporter::setFilename(std::string filename) {
    mFilename = filename;
}

FileExporter::FileExporter() {
    mFilename = "";
}

}