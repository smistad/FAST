#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 perspectiveTransform;
uniform float pointSize;

void main()
{
    gl_PointSize = pointSize;
    gl_Position = perspectiveTransform * viewTransform * transform * vec4(aPos, 1.0);
}
