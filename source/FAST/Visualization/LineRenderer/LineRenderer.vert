#version 330 core
layout (location = 0) in vec3 in_position;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 perspectiveTransform;

void main() {
    gl_Position = perspectiveTransform * viewTransform * transform * vec4(in_position, 1.0);
}
