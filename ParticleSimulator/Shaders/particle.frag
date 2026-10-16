#version 330 core

in vec3 particleColor;
out vec4 FragColor;

void main()
{
    vec2 centered = gl_PointCoord - vec2(0.5);

    if (dot(centered, centered) > 0.25)
        discard;

    FragColor = vec4(particleColor, 1.0);
}