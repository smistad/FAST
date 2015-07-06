#ifndef COLOR_HPP_
#define COLOR_HPP_

#include "FAST/Data/DataTypes.hpp"

namespace fast {

class Color : public Vector3f {
    public:
        Color() : Vector3f(0,0,0) {};
        Color(float red, float green, float blue) : Vector3f(red, green, blue) {};
        float getRedValue() const {
            return x();
        }
        float getGreenValue() const {
            return y();
        }
        float getBlueValue() const {
            return z();
        }
        static Color Red() {
            return Color(1,0,0);
        }
        static Color Green() {
            return Color(0,1,0);
        }
        static Color Blue() {
            return Color(0,0,1);
        }
        static Color White() {
            return Color(1,1,1);
        }
        static Color Black() {
            return Color(0,0,0);
        }
        static Color Yellow() {
            return Color(1, 1, 0);
        }

};


}

#endif
