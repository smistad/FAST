#include "Renderer.hpp"

using namespace fast;

Renderer::Renderer() {
    mWindow = -1;
    mLevel = -1;
}

void Renderer::setIntensityLevel(float level) {
    mLevel = level;
}

float Renderer::getIntensityLevel() {
    return mLevel;
}

void Renderer::setIntensityWindow(float window) {
    if(window <= 0)
        throw Exception("Intensity window has to be above 0.");
    mWindow = window;
}

float Renderer::getIntensityWindow() {
    return mWindow;
}



