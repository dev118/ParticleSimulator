
#pragma once

#include <glad/gl.h>
#include <vector>

#include "ParticleSystem.h"
#include "GpuParticleSystem.h"

class ParticleRenderer
{
public:
    bool Initialize();

    // Existing CPU rendering path.
    void Render(
        const std::vector<Particle>& particles,
        float cameraX,
        float cameraY,
        float zoom,
        int screenWidth,
        int screenHeight,
        float particleRadius
    );

    // New GPU rendering path.
    void RenderGPU(
        const GpuParticleSystem& particles,
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
    GLuint gpuParticleProgram = 0;

    GLuint VAO = 0;
    GLuint VBO = 0;

    GLint cameraPositionLocation = -1;
    GLint zoomLocation = -1;
    GLint screenSizeLocation = -1;
    GLint particleRadiusLocation = -1;

    GLint gpuCameraPositionLocation = -1;
    GLint gpuZoomLocation = -1;
    GLint gpuScreenSizeLocation = -1;
    GLint gpuParticleRadiusLocation = -1;

    std::vector<float> vertexData;
};