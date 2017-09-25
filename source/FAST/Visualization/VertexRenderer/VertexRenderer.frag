#version 330 core

in vec4 vertex_color;

void main()
{
    vec2 circCoord = gl_PointCoord - vec2(0.5, 0.5);
    if(dot(circCoord, circCoord) > 0.25) {
        discard;
    } else {
        gl_FragColor = vertex_color;
    }
}
