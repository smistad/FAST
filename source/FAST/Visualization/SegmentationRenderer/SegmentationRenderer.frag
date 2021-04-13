#version 330 core
layout (std140) uniform Colors {
    vec4 color[256];
};
out vec4 FragColor;

in vec2 TexCoord;

uniform usampler2D ourTexture;
uniform float opacity;
uniform float borderOpacity;
uniform int borderRadius;

void main()
{
    uint in_label = texture(ourTexture, vec2(TexCoord.x, 1.0 - TexCoord.y)).r;
    FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    vec2 size = textureSize(ourTexture, 0);

    if(in_label > uint(0) && in_label < uint(256)) {
        FragColor = color[in_label];

        // Border check
        bool isBorder = false;
        if(borderOpacity != opacity) {
            // Check neighbors
            // If any neighbors have a different label, we are at the border
            for(int a = -borderRadius; a <= borderRadius; ++a) {
                for(int b = -borderRadius; b <= borderRadius; ++b) {
                    if(uint(texture(ourTexture, vec2(TexCoord.x + float(a)/size.x, 1.0 - (TexCoord.y + float(b)/size.y))).r) != in_label) {
                        if(borderRadius == 1 || length(vec2(a, b)) < borderRadius) {
                            isBorder = true;
                        }
                    }
                }
            }
        }
        FragColor.a = isBorder ? borderOpacity : opacity;
    }
}