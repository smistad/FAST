#version 330

layout (lines) in;                              // now we can access 2 vertices
layout (triangle_strip, max_vertices = 4) out;  // always (for now) producing 2 triangles (so 4 vertices)
in vec4 vertexColor[];
out vec4 geomColor;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 perspectiveTransform;
uniform float thickness;
//uniform int viewportWidth;
//uniform int viewportHeight;

void main()
{
    geomColor = vertexColor[0];
    vec4 p1 = gl_in[0].gl_Position;
    vec4 p2 = gl_in[1].gl_Position;

    float size = (thickness/100.0)/(perspectiveTransform[0][0]/2.0); // percent to mm
    vec2 dir    = normalize((p2.xy - p1.xy));
    vec4 offset = vec4(-dir.y, dir.x, 0, 0) * size * 0.5;

    gl_Position = perspectiveTransform * viewTransform * transform * (p1 + offset);
    EmitVertex();
    gl_Position = perspectiveTransform * viewTransform * transform * (p1 - offset);
    EmitVertex();
    gl_Position = perspectiveTransform * viewTransform * transform * (p2 + offset);
    EmitVertex();
    gl_Position = perspectiveTransform * viewTransform * transform * (p2 - offset);
    EmitVertex();

    EndPrimitive();
}