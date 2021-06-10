#include "FileExporter.hpp"

namespace fast {

void FileExporter::setFilename(std::string filename) {
    m_filename = filename;
    setModified(true);
}

FileExporter::FileExporter() {
    m_filename = "";
    createStringAttribute("filename", "Filename", "Filename to export to", "");
}

void FileExporter::loadAttributes() {
    setFilename(getStringAttribute("filename")) ;
}

}