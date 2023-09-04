#pragma once

#include "FAST/Data/DataTypes.hpp"
#include <cctype> // std::tolower
#include <FAST/Utility.hpp>

namespace fast {

class FAST_EXPORT  Color {
    private:
        Vector3f mColorVector;
        bool m_null = false;
        std::string m_name;
        float m_alpha = -1.0f;
    public:
        Color() : mColorVector(Vector3f(0, 0, 0)), m_null(true) {};

        Color(float red, float green, float blue, float opacity = -1.0f, std::string name = "") : mColorVector(Vector3f(red, green, blue)), m_null(false), m_name(name), m_alpha(opacity) {};

        /**
         * Whether the color is set or not
         * @return
         */
        bool isNull() const {
            return m_null;
        }

        bool hasOpacity() const {
            return m_alpha >= 0.0f;
        }

        static Color fromString(std::string str) {
            if(str.find(';') != std::string::npos) { // CHeck if on the form RED;GREEN;BLUE, where these are integers from 0 to 255
                auto parts = split(str, ";");
                if(parts.size() != 3)
                    throw Exception("Parse error of color from string: " + str);
                float red = std::stoi(parts[0]) / 255.0f;
                float green = std::stoi(parts[1]) / 255.0f;
                float blue = std::stoi(parts[2]) / 255.0f;
                return Color(red, green, blue);
            }

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
        float getOpacity() const {
            return m_alpha;
        }
        std::string getName() const {
            if(m_name.empty()) {
                Vector3i intVector = (mColorVector*255).cast<int>();
                return std::to_string(intVector.x()) + ";" +
                        std::to_string(intVector.y()) + ";" +
                        std::to_string(intVector.z());
            }
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
            return Color(1,0,0, -1, "red");
        }
        static Color Green() {
            return Color(0,1,0, -1, "green");
        }
        static Color Blue() {
            return Color(0,0,1, -1, "blue");
        }
        static Color White() {
            return Color(1,1,1, -1, "white");
        }
        static Color Black() {
            return Color(0,0,0, -1, "black");
        }
        static Color Yellow() {
            return Color(1, 1, 0, -1, "yellow");
        }
        static Color Magenta() {
            return Color(1, 0, 1, -1, "magenta");
        }
        static Color Cyan() {
            return Color(0, 1, 1, -1, "cyan");
        }
        static Color Brown() {
            return Color(1, 0.6, 0.2, -1, "brown");
        }

};

using LabelColors = std::map<uint, Color>;

}

