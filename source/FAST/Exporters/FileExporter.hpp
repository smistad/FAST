#ifndef FILE_EXPORTER_HPP_
#define FILE_EXPORTER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {
    /*
     * Abstract class for file exporters
     */
   class FileExporter : public ProcessObject {
   public:
       virtual void setFilename(std::string filename);
   protected:
       std::string mFilename;
   protected:
       FileExporter();
       void execute() = 0;

   };
}

#endif