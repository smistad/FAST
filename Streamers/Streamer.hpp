#ifndef STREAMER_HPP_
#define STREAMER_HPP_

#include "PipelineObject.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

namespace fast {

class Streamer : public PipelineObject {
    public:
        typedef boost::shared_ptr<Streamer> Ptr;
        boost::mutex& getStreamMutex() {return mStreamMutex;};
        virtual void producerStream() = 0;
        virtual ~Streamer() {};
    protected:

        // Mutex used to synchronize the producer running in a secondary thread,
        // and the consumers working in the main thread
        boost::mutex mStreamMutex;

};

} // end namespace fast

#endif /* STREAMER_HPP_ */
