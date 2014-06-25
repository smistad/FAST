#ifndef HISTOGRAMPYRAMIDS_HPP_
#define HISTOGRAMPYRAMIDS_HPP_

#include "Context.hpp"
#include <vector>

namespace oul {

/**
 * Abstract class for all HistogramPyramids
 */
class HistogramPyramid {
    public:
        static void compileCode(oul::Context &context);
        int getSum();
        virtual cl::Buffer createPositionBuffer() = 0;
        virtual void deleteHPlevels() = 0;
    protected:
        oul::Context context; //this will call the default constructor in Context
        int size;
        int sum;
};

/**
 * 2D Histogram pyramid
 */
class HistogramPyramid2D : public HistogramPyramid {
    public:
        HistogramPyramid2D(oul::Context &context);
        void create(cl::Image2D &image, int, int);
        cl::Buffer createPositionBuffer();
        void deleteHPlevels();
        void traverse(cl::Kernel &kernel, int);
    private:
        std::vector<cl::Image2D> HPlevels;
};

/**
 * 3D Histogram pyramid (that uses 3D textures)
 */
class HistogramPyramid3D : public HistogramPyramid {
    public:
        HistogramPyramid3D(oul::Context &context);
        void create(cl::Image3D &image, int, int, int);
        cl::Buffer createPositionBuffer();
        void deleteHPlevels();
        void traverse(cl::Kernel &kernel, int);
    private:
        std::vector<cl::Image3D> HPlevels;
};

/**
 * 3D Histogram pyramid (which uses buffers instead of textures)
 */
class HistogramPyramid3DBuffer : public HistogramPyramid {
    public:
        HistogramPyramid3DBuffer(oul::Context &context);
        void create(cl::Buffer &buffer, int, int, int);
        cl::Buffer createPositionBuffer();
        void deleteHPlevels();
        void traverse(cl::Kernel &kernel, int);
    private:
        int sizeX,sizeY,sizeZ;
        std::vector<cl::Buffer> HPlevels;
};
} // end namespace

#endif /* HISTOGRAMPYRAMIDS_HPP_ */
