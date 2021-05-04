#include "FileExporter.hpp"

namespace fast {

void FileExporter::setFilename(std::string filename) {
    mFilename = filename;
}

FileExporter::FileExporter() {
    mFilename = "";
    createStringAttribute("filename", "Filename", "Filename to export to", "");
}

void FileExporter::loadAttributes() {
    setFilename(getStringAttribute("filename")) ;
}

}