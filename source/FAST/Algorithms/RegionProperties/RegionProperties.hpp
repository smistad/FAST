#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/SimpleDataObject.hpp>
namespace fast {

class Mesh;

struct FAST_EXPORT Region {
    int area;
    uchar label;
    Vector2f centroid;
    SharedPointer<Mesh> contour;
    std::vector<Vector2i> pixels;
};

FAST_SIMPLE_DATA_OBJECT(RegionList, std::vector<Region>)

class FAST_EXPORT RegionProperties : public ProcessObject {
    FAST_OBJECT(RegionProperties)
    public:
    protected:
        RegionProperties();
        void execute() override;
};

}