#ifndef ImageExporter_HPP_
#define ImageExporter_HPP_

#include "FAST/ProcessObject.hpp"
#include <string>

namespace fast {

class FAST_EXPORT  ImageExporter : public ProcessObject {
    FAST_OBJECT(ImageExporter)
    public:
        void setFilename(std::string filename);
    private:
        ImageExporter();
        void execute();

        std::string mFilename;
};


} // end namespace fast




#endif /* ImageExporter_HPP_ */
