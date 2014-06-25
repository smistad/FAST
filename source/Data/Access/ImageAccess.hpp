#ifndef IMAGEACCESS_HPP_
#define IMAGEACCESS_HPP_

namespace fast {

class ImageAccess {
    public:
        ImageAccess(void* data, bool* accessFlag, bool* accessFlag2);
        void* get();
        void release();
        ~ImageAccess();
    private:
        void* mData;
        bool* mAccessFlag;
        bool* mAccessFlag2;
};

} // end namespace fast



#endif /* IMAGEACCESS2D_HPP_ */
