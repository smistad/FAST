#ifndef IMAGEACCESS2D_HPP_
#define IMAGEACCESS2D_HPP_

namespace fast {

class ImageAccess2D {
    public:
        ImageAccess2D(void * data);
        void * get();
    private:
        void * mData;
};

} // end namespace fast



#endif /* IMAGEACCESS2D_HPP_ */
