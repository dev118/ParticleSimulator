
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 particleColor;

uniform vec2 cameraPosition;
uniform float zoom;
uniform vec2 screenSize;
uniform float particleRadius;

void main()
{
    vec2 worldPosition = aPos - cameraPosition;

    vec2 normalizedPosition =
        worldPosition * zoom * 2.0 / screenSize;

    gl_Position = vec4(normalizedPosition, 0.0, 1.0);

    // Convert world-space radius to a screen-space diameter.
    gl_PointSize = particleRadius * 2.0 * zoom;

    particleColor = aColor;
}