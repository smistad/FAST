#pragma once

#include <FAST/Exporters/Exporter.hpp>

namespace fast {
/**
 * @brief Abstract class for file exporters
 */
class FileExporter : public Exporter {
   public:
       virtual void setFilename(std::string filename);
   protected:
       std::string mFilename;
   protected:
       FileExporter();
       void execute() = 0;

   };
}
