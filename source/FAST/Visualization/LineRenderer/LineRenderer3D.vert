#version 330 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

out vec4 vertexColor;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 perspectiveTransform;
uniform bool useGlobalColor;
uniform vec3 globalColor;

void main() {
    gl_Position = perspectiveTransform * viewTransform * transform * vec4(in_position, 1.0);
    if(useGlobalColor) {
        vertexColor = vec4(globalColor, 1.0);
    } else {
        vertexColor = vec4(in_color, 1.0);
    }
}
