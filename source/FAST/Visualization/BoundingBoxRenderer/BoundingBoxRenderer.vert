#version 330 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in uint in_label;
layout (std140) uniform Colors {
    vec4 color[256];
};

out vec4 vertexColor;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 perspectiveTransform;

void main() {
    gl_Position = perspectiveTransform * viewTransform * transform * vec4(in_position, 1.0);
    vertexColor = in_label < uint(256) ? color[in_label] : vec4(0.0, 0.0, 0.0, 0.0);
}
