#ifndef STREAMER_HPP_
#define STREAMER_HPP_

#include "PipelineObject.hpp"
namespace fast {

class Streamer : public PipelineObject {
    private:
        virtual void producerStream() = 0;

};

}; // end namespace fast




#endif /* STREAMER_HPP_ */
