#ifndef STREAMER_HPP_
#define STREAMER_HPP_

#include "ProcessObject.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

namespace fast {

class Streamer : public ProcessObject {
    public:
        typedef boost::shared_ptr<Streamer> Ptr;
        virtual void producerStream() = 0;
        virtual ~Streamer() {};
    protected:


};

} // end namespace fast

#endif /* STREAMER_HPP_ */
