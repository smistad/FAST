#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec3 LightPos;
in vec3 ViewPos;
in vec3 Color;

out vec4 FragColor;

uniform bool opacity;

void main()
{
    vec3 objectColor = Color;
    vec3 lightColor = vec3(0.7, 0.7, 0.7);
    vec3 ambient = vec3(0.2, 0.2, 0.2);
    float specularStrength = 0.5;
    float shininess = 16;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    vec3 viewDir = normalize(ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, opacity);
}
