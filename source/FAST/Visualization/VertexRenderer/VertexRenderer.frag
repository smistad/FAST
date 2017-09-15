#version 330 core

void main()
{
    vec2 circCoord = gl_PointCoord - vec2(0.5, 0.5);
    if(dot(circCoord, circCoord) > 0.25) {
        discard;
    } else {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
}
