#version 330 core
layout (std140) uniform Colors {
    vec4 color[256];
};
out vec4 FragColor;

in vec2 TexCoord;

// Using sampler2D here instead of usampler2d, due to texture compression
uniform sampler2D ourTexture;
uniform float opacity;
uniform float borderOpacity;
uniform int borderRadius;

void main()
{
    uint in_label = uint(round(texture(ourTexture, vec2(TexCoord.x, TexCoord.y)).r*255)); // Texture compression is normalized
    FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    vec2 size = textureSize(ourTexture, 0);
    uint borderLabel = uint(0);

    // Border check
    bool isBorder = false;
    if(borderRadius > 1 || borderOpacity != opacity) {
        // Check neighbors
        // If any neighbors have a different label, we are at the border
        for(int a = -borderRadius; a <= borderRadius; ++a) {
            for(int b = -borderRadius; b <= borderRadius; ++b) {
                if(uint(round(texture(ourTexture, vec2(TexCoord.x + float(a)/size.x, (TexCoord.y + float(b)/size.y))).r*255)) != in_label){
                    if(borderRadius == 1 || length(vec2(a, b)) < borderRadius) {
                				if(in_label == uint(0)) {
                  					// Outside
                            if(borderRadius > 1) {
  						                     borderLabel = uint(round(texture(ourTexture, vec2(TexCoord.x + float(a)/size.x, (TexCoord.y + float(b)/size.y))).r*255));
  						                     isBorder = true;
  					                }
			                  } else {
                    				// Inside
                    				isBorder = true;
                  			}
                		}
                }
            }
        }
    }

    if((in_label > uint(0) && in_label < uint(256)) || isBorder) {
        if(isBorder && in_label == uint(0))
            in_label = borderLabel;
        FragColor = color[in_label];
        FragColor.a = isBorder ? borderOpacity : opacity;
    }
}
