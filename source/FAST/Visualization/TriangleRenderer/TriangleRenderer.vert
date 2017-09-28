#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 Normal;
out vec3 FragPos;
out vec3 LightPos;
out vec3 ViewPos;
out vec3 Color;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 perspectiveTransform;
uniform bool use_normals;
uniform bool useGlobalColor;
uniform vec3 globalColor;

void main()
{
    gl_Position = perspectiveTransform * viewTransform * transform * vec4(aPos, 1.0);
    if(use_normals) {
        Normal = mat3(transpose(inverse(transform))) * aNormal;
    } else {
        Normal = vec3(1,0,0);
    }
    FragPos = vec3(transform * vec4(aPos, 1.0));
    LightPos = vec3(inverse(viewTransform) * vec4(0.0, 0.0, 0.0, 1.0));
    ViewPos = vec3(inverse(viewTransform) * vec4(0.0, 0.0, 0.0, 1.0));
    if(useGlobalColor) {
        Color = globalColor;
    } else {
        Color = aColor;
    }
}
