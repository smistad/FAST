#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec4 vertex_color;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 perspectiveTransform;
uniform float pointSize;

void main()
{
    gl_PointSize = pointSize;
    gl_Position = perspectiveTransform * viewTransform * transform * vec4(aPos, 1.0);
    vertex_color = vec4(aColor, 1.0);
}
