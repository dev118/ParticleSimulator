
#pragma once

#include <glad/gl.h>
#include <vector>

#include "ParticleSystem.h"

class ParticleRenderer
{
public:
    bool Initialize();

    void Render(
        const std::vector<Particle>& particles,
        float cameraX,
        float cameraY,
        float zoom,
        int screenWidth,
        int screenHeight,
        float particleRadius
    );

    void Shutdown();

private:
    GLuint shaderProgram = 0;
    GLuint VAO = 0;
    GLuint VBO = 0;

    GLint cameraPositionLocation = -1;
    GLint zoomLocation = -1;
    GLint screenSizeLocation = -1;
    GLint particleRadiusLocation = -1;

    std::vector<float> vertexData;
};