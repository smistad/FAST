#version 330 core

out vec4 FragColor;
in vec4 vertex_color;

void main()
{
    // Convert squares to circles
    vec2 circCoord = gl_PointCoord - vec2(0.5, 0.5);
    if(dot(circCoord, circCoord) > 0.25) { // x dot x = |x|*|x| = r^2, radius = 0.5, r^2=0.25
        discard;
    } else {
        FragColor = vertex_color;
    }
}
