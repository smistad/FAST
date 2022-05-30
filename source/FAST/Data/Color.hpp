#pragma once

#include "FAST/Data/DataTypes.hpp"
#include <cctype> // std::tolower

namespace fast {

class FAST_EXPORT  Color {
    private:
        Vector3f mColorVector;
        bool m_null = false;
        std::string m_name;
    public:
        Color() : mColorVector(Vector3f(0, 0, 0)), m_null(true) {};

        Color(float red, float green, float blue, std::string name = "") : mColorVector(Vector3f(red, green, blue)), m_null(false), m_name(name) {};

        /**
         * Whether the color is set or not
         * @return
         */
        bool isNull() const {
            return m_null;
        }

        static Color fromString(std::string str) {
            std::transform(str.begin(), str.end(), str.begin(),
                [](unsigned char c) { return std::tolower(c); });
            std::map<std::string, Color> mapping = {
                {"black", Color::Black()},
                {"white", Color::White()},
                {"red", Color::Red()},
                {"blue", Color::Blue()},
                {"green", Color::Green()},
                {"yellow", Color::Yellow()},
                {"cyan", Color::Cyan()},
                {"magenta", Color::Magenta()},
                {"brown", Color::Brown()},
            };

            if(mapping.count(str) == 0)
                throw Exception("Could not find color " + str);

            return mapping[str];
        }

        Vector3f asVector() const {
            return mColorVector;
        }
        float getRedValue() const {
            return mColorVector.x();
        }
        float getGreenValue() const {
            return mColorVector.y();
        }
        float getBlueValue() const {
            return mColorVector.z();
        }
        std::string getName() const {
            return m_name;
        }
        /**
         * The color Null means NO COLOR, or not set, resulting in isNull() returning true
         * @return
         */
        static Color Null() {
            return Color();
        }
        static Color Red() {
            return Color(1,0,0, "red");
        }
        static Color Green() {
            return Color(0,1,0, "green");
        }
        static Color Blue() {
            return Color(0,0,1, "blue");
        }
        static Color White() {
            return Color(1,1,1, "white");
        }
        static Color Black() {
            return Color(0,0,0, "black");
        }
        static Color Yellow() {
            return Color(1, 1, 0, "yellow");
        }
        static Color Magenta() {
            return Color(1, 0, 1, "magenta");
        }
        static Color Cyan() {
            return Color(0, 1, 1, "cyan");
        }
        static Color Brown() {
            return Color(1, 0.6, 0.2, "brown");
        }

};

using LabelColors = std::map<uint, Color>;

}

