#ifndef IMAGEACCESS2D_HPP_
#define IMAGEACCESS2D_HPP_

namespace fast {

class ImageAccess2D {
    public:
        ImageAccess2D(void * data, bool * accessFlag);
        void * get();
        ~ImageAccess2D();
    private:
        void * mData;
        bool * mAccessFlag;
};

} // end namespace fast



#endif /* IMAGEACCESS2D_HPP_ */
