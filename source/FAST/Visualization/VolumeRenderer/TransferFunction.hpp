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
                300, 1, 0, 0, 0.25f,
                500, 1, 1, 0.8f, 1,
            });
        }

        static TransferFunction CT_Bone() {
            return TransferFunction({
                150, 0, 0, 0, 0,
                600, 1, 1, 0.5f, 0.1f,
                1000, 1, 1, 0.8f, 1,
            });
        }

        static TransferFunction CT_Blood() {
            return TransferFunction({
                100, 1, 0, 0, 0,
                300, 1, 0.1f, 0.1f, 0.5f,
                300, 0, 0, 0, 0,
            });
        }

        static TransferFunction Ultrasound() {
            return TransferFunction({
                100, 0, 0, 0, 0,
                200, 0.8f, 0.8f, 1.0f, 0.1f,
                220, 1.0f, 0.7f, 0.2f, 0.25f,
                255, 1.0f, 1.0f, 1.0f, 1.0f,
            });
        }
    protected:
        bool checkData() const;
        std::vector<float> m_data;

};

}