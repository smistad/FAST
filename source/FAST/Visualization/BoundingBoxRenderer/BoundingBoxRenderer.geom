#version 330 core

layout (lines) in;
layout (triangle_strip, max_vertices=4) out;
in vec4 vertexColor[];
out vec4 geomColor;

//uniform float size;
uniform mat4 perspectiveTransform;
uniform float borderSize;

void main() {
    geomColor = vertexColor[0];
    vec4 first = gl_in[0].gl_Position;
    vec4 second = gl_in[1].gl_Position;
    float pixelWidth = perspectiveTransform[0][0];
    float pixelHeight = perspectiveTransform[1][1];
    float sizeX = borderSize*pixelWidth;
    float sizeY = borderSize*pixelHeight;

    // Draw bounding box with triangle strips
    if(first.x == second.x) {
        // Vertical line
        if(first.y < second.y) {
            gl_Position = first + vec4(sizeX, -sizeY, 0, 0);
            EmitVertex();
            gl_Position = second + vec4(sizeX, sizeY, 0, 0);
            EmitVertex();
            gl_Position = first + vec4(-sizeX, -sizeY, 0, 0);
            EmitVertex();
            gl_Position = second + vec4(-sizeX, sizeY, 0, 0);
            EmitVertex();
            EndPrimitive();
        } else {
            gl_Position = first + vec4(-sizeX, sizeY, 0, 0);
            EmitVertex();
            gl_Position = second + vec4(-sizeX, -sizeY, 0, 0);
            EmitVertex();
            gl_Position = first + vec4(sizeX, sizeY, 0, 0);
            EmitVertex();
            gl_Position = second + vec4(sizeX, -sizeY, 0, 0);
            EmitVertex();
            EndPrimitive();
        }
    } else {
        // Horizontal line
        if(first.x < second.x) {
            gl_Position = first + vec4(-sizeX, -sizeY, 0, 0);
            EmitVertex();
            gl_Position = second + vec4(sizeX, -sizeY, 0, 0);
            EmitVertex();
            gl_Position = first + vec4(-sizeX, sizeY, 0, 0);
            EmitVertex();
            gl_Position = second + vec4(sizeX, sizeY, 0, 0);
            EmitVertex();
            EndPrimitive();
        } else {
            gl_Position = second + vec4(sizeX, sizeY, 0, 0);
            EmitVertex();
            gl_Position = first + vec4(-sizeX, sizeY, 0, 0);
            EmitVertex();
            gl_Position = second + vec4(sizeX, -sizeY, 0, 0);
            EmitVertex();
            gl_Position = first + vec4(-sizeX, -sizeY, 0, 0);
            EmitVertex();
            EndPrimitive();
        }
    }
}