#pragma once

#include <FAST/Data/DataTypes.hpp>
#include <FAST/Data/Color.hpp>

namespace fast {

class FAST_EXPORT TransferFunction {
    public:
        TransferFunction() {};
        TransferFunction(std::initializer_list<float> values);
        int addPoint(float intensity, Vector4f color);
        void removePoint(int i);
        void clear();
        int getSize() const;
        std::tuple<float, Vector4f> getPoint(int i) const;
        cl::Buffer getAsOpenCLBuffer(OpenCLDevice::pointer device) const;

        static TransferFunction CT_Blood_And_Bone() {
            return TransferFunction({
                100, 0, 0, 0, 0,
                300, 1, 0, 0, 0.25,
                500, 1.0, 1.0, 0.8, 1,
            });
        }

        static TransferFunction CT_Bone() {
            return TransferFunction({
                150, 0, 0, 0, 0,
                600, 1, 1, 0.5, 0.1,
                1000, 1.0, 1.0, 0.8, 1,
            });
        }

        static TransferFunction CT_Blood() {
            return TransferFunction({
                100, 1, 0, 0, 0,
                300, 1, 0.1, 0.1, 0.5,
                300, 0, 0, 0, 0,
            });
        }

        static TransferFunction Ultrasound() {
            return TransferFunction({
                100, 0, 0, 0, 0,
                200, 0.8, 0.8, 1.0, 0.1,
                220, 1.0, 0.7, 0.2, 0.25,
                255, 1.0, 1.0, 1.0, 1.0,
            });
        }
    protected:
        bool checkData() const;
        std::vector<float> m_data;

};

}