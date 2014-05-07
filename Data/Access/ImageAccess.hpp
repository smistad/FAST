#ifndef IMAGEACCESS_HPP_
#define IMAGEACCESS_HPP_

namespace fast {

class ImageAccess {
    public:
        ImageAccess(void * data, bool * accessFlag);
        void * get();
        ~ImageAccess();
    private:
        void * mData;
        bool * mAccessFlag;
};

} // end namespace fast



#endif /* IMAGEACCESS2D_HPP_ */
