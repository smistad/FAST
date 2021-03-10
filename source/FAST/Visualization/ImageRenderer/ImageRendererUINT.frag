#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform usampler2D ourTexture;

void main()
{
    // TODO Divide by higher number if UINT16
    FragColor = vec4(texture(ourTexture, vec2(TexCoord.x, 1.0 - TexCoord.y)))/255.0;
}