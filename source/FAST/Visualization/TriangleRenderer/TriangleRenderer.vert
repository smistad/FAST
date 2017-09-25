#version 330 core
layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;
out vec3 LightPos;
out vec3 ViewPos;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 perspectiveTransform;

void main()
{
    gl_Position = perspectiveTransform * viewTransform * transform * vec4(aPos, 1.0);
    Normal = vec3(1,0,0);//mat3(transpose(inverse(transform))) * aNormal;
    FragPos = vec3(transform * vec4(aPos, 1.0));
    LightPos = vec3(inverse(viewTransform) * vec4(0.0, 0.0, 0.0, 1.0));
    ViewPos = vec3(inverse(viewTransform) * vec4(0.0, 0.0, 0.0, 1.0));
}
