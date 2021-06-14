#version 330 core

in vec4 geomColor;
out vec4 fragColor;

void main() {
    fragColor = geomColor;
}