#ifndef OPENCL_PROGRAM_HPP_
#define OPENCL_PROGRAM_HPP_

#include "Object.hpp"
#include "SmartPointers.hpp"
#include <unordered_map>

namespace cl {

class Program;

}

namespace fast {

class OpenCLDevice;

class FAST_EXPORT  OpenCLProgram : public Object {
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
        std::unordered_map<SharedPointer<OpenCLDevice>, std::map<std::string, cl::Program> > mOpenCLPrograms;
};

} // end namespace fast

#endif
