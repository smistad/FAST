#pragma once

#include <FAST/Exporters/Exporter.hpp>

namespace fast {
/**
 * @brief Abstract class for file exporters
 */
class FAST_EXPORT FileExporter : public Exporter {
   public:
       virtual void setFilename(std::string filename);
       void loadAttributes();
   protected:
       std::string mFilename;
   protected:
       FileExporter();
       void execute() = 0;

   };
}
