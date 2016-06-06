#ifndef OPENCL_PROGRAM_HPP_
#define OPENCL_PROGRAM_HPP_

#include "Object.hpp"
#include "SmartPointers.hpp"
#include <boost/unordered_map.hpp>

namespace cl {

class Program;

}

namespace fast {

class OpenCLDevice;

class OpenCLProgram : public Object {
    FAST_OBJECT(OpenCLProgram)
    public:
        void setName(std::string name);
        std::string getName() const;
        void setSourceFilename(std::string filename);
        std::string getSourceFilename() const;
        cl::Program build(SharedPointer<OpenCLDevice>, std::string buildOptions = "");
    protected:
        OpenCLProgram();

        bool buildExists(SharedPointer<OpenCLDevice>, std::string buildOptions = "") const;

        std::string mName;
        std::string mSourceFilename;
        boost::unordered_map<SharedPointer<OpenCLDevice>, boost::unordered_map<std::string, cl::Program> > mOpenCLPrograms;
};

} // end namespace fast

#endif
