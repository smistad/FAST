#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform usampler2D ourTexture;
uniform float window;
uniform float level;
uniform float opacity;

void main() {
    vec4 color = vec4(texture(ourTexture, vec2(TexCoord.x, 1.0 - TexCoord.y)));
    color.rgb = (color.rgb - level + window/2.0f) / window; // Normalize RGB
    color.a = color.a/255.0; // Normalize alpha
    if(opacity >= 0.0)
        color.w = opacity;
    FragColor = clamp(color, 0.0f, 1.0f);
}