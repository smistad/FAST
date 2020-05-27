#pragma once
#include "FAST/Data/DataTypes.hpp"

namespace fast {

class Image;

class FAST_EXPORT  ImageAccess {
    public:
        ImageAccess(void* data, SharedPointer<Image> image);
        void* get();
        template <class T>
        T getScalarFast(uint position, uchar channel = 0) const noexcept;
        template <class T>
        T getScalarFast(VectorXi, uchar channel = 0) const noexcept;
        template <class T>
        T getScalarFast2D(Vector2i, uchar channel = 0) const noexcept;
        template <class T>
        T getScalarFast3D(Vector3i, uchar channel = 0) const noexcept;
        float getScalar(uint position, uchar channel = 0) const;
        float getScalar(VectorXi position, uchar channel = 0) const;
        Vector4f getVector(VectorXi position) const;
        template <class T>
        void setScalarFast(uint position, T value, uchar channel = 0) noexcept;
        template <class T>
        void setScalarFast(VectorXi position, T value, uchar channel = 0) noexcept;
        template <class T>
        void setScalarFast2D(Vector2i position, T value, uchar channel = 0) noexcept;
        template <class T>
        void setScalarFast3D(Vector3i position, T value, uchar channel = 0) noexcept;
        void setScalar(uint position, float value, uchar channel = 0);
        void setScalar(VectorXi position, float value, uchar channel = 0);
		void setVector(uint position, Vector4f value);
        void setVector(VectorXi position, Vector4f value);
        void release();
        ~ImageAccess();
		typedef std::unique_ptr<ImageAccess> pointer;
    private:
		ImageAccess(const ImageAccess::pointer other) = delete;
		ImageAccess::pointer operator=(const ImageAccess::pointer other) = delete;
        void* mData;
        const int m_width, m_height, m_depth, m_channels, m_dimensions;

        SharedPointer<Image> mImage;
};

template <class T>
T ImageAccess::getScalarFast(uint position, uchar channel) const noexcept {
    return ((T*)mData)[position * m_channels + channel];
}

template <class T>
T ImageAccess::getScalarFast(VectorXi position, uchar channel) const noexcept {
    if(m_dimensions == 2) {
        return ((T*)mData)[(position.x() + position.y() * m_width) * m_channels + channel];
    } else {
        return ((T*)mData)[(position.x() + position.y() * m_width + position.z()*m_width*m_height) * m_channels + channel];
    }
}

template <class T>
T ImageAccess::getScalarFast2D(Vector2i position, uchar channel) const noexcept {
	return ((T*)mData)[(position.x() + position.y() * m_width) * m_channels + channel];
}

template <class T>
T ImageAccess::getScalarFast3D(Vector3i position, uchar channel) const noexcept {
	return ((T*)mData)[(position.x() + position.y() * m_width + position.z()*m_width*m_height) * m_channels + channel];
}

template <class T>
void ImageAccess::setScalarFast(uint position, T value, uchar channel) noexcept {
    ((T*)mData)[position * m_channels + channel] = value;
}

template <class T>
void ImageAccess::setScalarFast(VectorXi position, T value, uchar channel) noexcept {
	if(m_dimensions == 2) {
        ((T*)mData)[(position.x() + position.y() * m_width) * m_channels + channel] = value;
    } else {
        ((T*)mData)[(position.x() + position.y() * m_width + position.z()*m_width*m_height) * m_channels + channel] = value;
    }
}

template <class T>
void ImageAccess::setScalarFast2D(Vector2i position, T value, uchar channel) noexcept {
	((T*)mData)[(position.x() + position.y() * m_width) * m_channels + channel] = value;
}

template <class T>
void ImageAccess::setScalarFast3D(Vector3i position, T value, uchar channel) noexcept {
	((T*)mData)[(position.x() + position.y() * m_width + position.z()*m_width*m_height) * m_channels + channel] = value;
}


} // end namespace fast
