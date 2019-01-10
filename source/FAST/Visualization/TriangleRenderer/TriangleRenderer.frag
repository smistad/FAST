#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec3 LightPos;
in vec3 ViewPos;
in vec3 Color;

out vec4 FragColor;

uniform float opacity;
uniform bool mode2D;
uniform bool ignoreInvertedNormals;

void main()
{
    if(mode2D) {
        FragColor = vec4(Color, opacity);
    } else {
        vec3 objectColor = Color;
        vec3 lightColor = vec3(0.7, 0.7, 0.7);
        vec3 ambientColor = vec3(0.2, 0.2, 0.2);
        vec3 specularColor = vec3(1.0, 1.0, 1.0);
        float shininess = 16;

        // ambient
        vec3 ambient = lightColor * ambientColor;

        // diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(LightPos - FragPos);
        float diff;
        if(ignoreInvertedNormals) {
            diff = abs(dot(norm, lightDir));
        } else {
            diff = max(dot(norm, lightDir), 0.0);
        }
        vec3 diffuse = lightColor * (diff * objectColor);

        // specular
        vec3 viewDir = normalize(ViewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = lightColor * (spec * specularColor);

        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result, opacity);
    }
}
