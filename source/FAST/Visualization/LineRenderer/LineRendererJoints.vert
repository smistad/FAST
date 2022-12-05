#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec4 vertex_color;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 perspectiveTransform;
uniform float pointSize;
uniform bool useGlobalColor;
uniform vec3 globalColor;
uniform int viewportWidth;

void main()
{
    gl_Position = perspectiveTransform * viewTransform * transform * vec4(aPos, 1.0);
    gl_PointSize = (pointSize/100.0)*viewportWidth; // gl_PointSize unit is in pixels
    if(useGlobalColor) {
        vertex_color = vec4(globalColor, 1.0);
    } else {
        vertex_color = vec4(aColor, 1.0);
    }
}
