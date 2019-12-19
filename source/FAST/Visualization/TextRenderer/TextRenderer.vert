#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 perspectiveTransform;
uniform int positionType;

void main()
{
    if(positionType == 2) {
        gl_Position = perspectiveTransform * viewTransform * transform * vec4(aPos, 1.0);
    } else {
        gl_Position = transform * vec4(aPos, 1.0);
    }
    TexCoord = aTexCoord;
}