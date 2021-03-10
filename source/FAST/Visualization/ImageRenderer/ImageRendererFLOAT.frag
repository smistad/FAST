#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    FragColor = vec4(texture(ourTexture, vec2(TexCoord.x, 1.0 - TexCoord.y)));
}