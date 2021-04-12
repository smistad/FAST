#version 330 core
layout (std140) uniform Colors {
    vec4 color[256];
};
out vec4 FragColor;

in vec2 TexCoord;

uniform usampler2D ourTexture;
uniform float opacity;

void main()
{
    uint in_label = texture(ourTexture, vec2(TexCoord.x, TexCoord.y)).r;
    FragColor = in_label < uint(256) && in_label > uint(0) ? color[in_label] : vec4(0.0, 0.0, 0.0, 0.0);
    if(in_label > uint(0))
    FragColor.a = opacity;
}
