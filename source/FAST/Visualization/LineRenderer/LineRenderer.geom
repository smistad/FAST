#version 330

layout (lines) in;                              // now we can access 2 vertices
layout (triangle_strip, max_vertices = 4) out;  // always (for now) producing 2 triangles (so 4 vertices)
in vec4 vertexColor[];
out vec4 geomColor;

uniform mat4 perspectiveTransform;
uniform float thickness;
//uniform int viewportWidth;
//uniform int viewportHeight;

void main()
{
    geomColor = vertexColor[0];
    vec4 p1 = gl_in[0].gl_Position;
    vec4 p2 = gl_in[1].gl_Position;

    //vec2 viewportSize = vec2(viewportWidth,viewportHeight);//vec2(perspectiveTransform[0][0], perspectiveTransform[1][1]);
    vec2 size = vec2(thickness, thickness)/200; // Assumes thickness is given in percent.
    //if(size.x < 1.0/viewportSize.x || size.y < 1.0/viewportSize.y) // minimum size
    //    size = vec2(1, 1)/viewportSize;
    vec2 dir    = normalize((p2.xy - p1.xy));
    vec2 offset = vec2(-dir.y, dir.x) * size;

    gl_Position = p1 + vec4(offset.xy, 0.0, 0.0);
    EmitVertex();
    gl_Position = p1 - vec4(offset.xy, 0.0, 0.0);
    EmitVertex();
    gl_Position = p2 + vec4(offset.xy, 0.0, 0.0);
    EmitVertex();
    gl_Position = p2 - vec4(offset.xy, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}