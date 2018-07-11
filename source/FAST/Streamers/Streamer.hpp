#ifndef STREAMER_HPP_
#define STREAMER_HPP_

#include <FAST/ProcessObject.hpp>

#include "FAST/Data/DataTypes.hpp"
#include "FAST/Exception.hpp"

namespace fast {


class FAST_EXPORT  NoMoreFramesException : public Exception {
    public:
        NoMoreFramesException(std::string message) : Exception(message) {};
};

class FAST_EXPORT Streamer : public ProcessObject {
    public:
        typedef SharedPointer<Streamer> pointer;
        virtual ~Streamer() {};
        virtual bool hasReachedEnd() = 0;
        static std::string getStaticNameOfClass() {
            return "Streamer";
        }


};

} // end namespace fast

#endif /* STREAMER_HPP_ */
