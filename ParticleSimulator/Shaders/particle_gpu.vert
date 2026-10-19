#version 430 core

struct Particle
{
    vec2 position;
    vec2 velocity;
    vec4 color;
};

layout(std430, binding = 0) readonly buffer ParticleBuffer
{
    Particle particles[];
};

uniform vec2 cameraPosition;
uniform float zoom;
uniform vec2 screenSize;
uniform float particleRadius;

out vec3 particleColor;

void main()
{
    Particle p = particles[gl_VertexID];

    vec2 screenPosition =
        (p.position - cameraPosition) * zoom
        + screenSize * 0.5;

    vec2 clipPosition =
        (screenPosition / screenSize) * 2.0 - 1.0;

    gl_Position = vec4(clipPosition, 0.0, 1.0);

    gl_PointSize = particleRadius * 2.0 * zoom;

    particleColor = p.color.rgb;
}