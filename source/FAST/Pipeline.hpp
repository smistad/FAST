#ifndef FAST_PIPELINE_HPP_
#define FAST_PIPELINE_HPP_

#include <string>
#include <vector>

namespace fast {

template <class T>
class SharedPointer;
class ProcessObjectPort;
class Renderer;

class Pipeline {
    public:
        Pipeline(std::string name, std::string description, std::string filename);
        std::vector<SharedPointer<Renderer> > setup(ProcessObjectPort input);
    private:
        std::string mName;
    public:
        std::string getName() const;
        std::string getDescription() const;
        std::string getFilename() const;

    private:
        std::string mDescription;
        std::string mFilename;
};

/**
 * Retrieve a list of all pipelines stored in .fpl files in the specified pipeline directory
 * @return
 */
std::vector<Pipeline> getAvailablePipelines();

} // end namespace fast

#endif
