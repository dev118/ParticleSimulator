#version 430 core

in vec3 particleColor;

out vec4 FragColor;

void main()
{
    vec2 point = gl_PointCoord * 2.0 - 1.0;

    if (dot(point, point) > 1.0)
    {
        discard;
    }

    FragColor = vec4(particleColor, 1.0);
}