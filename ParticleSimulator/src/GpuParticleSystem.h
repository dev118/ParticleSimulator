#pragma once

#include "ParticleSystem.h"
#include <glad/gl.h>
#include <cstddef>

struct alignas(16) ParticleGPU
{
    float x, y;
    float vx, vy;
    float r, g, b, a;
};

class GpuParticleSystem
{
public:
    GpuParticleSystem() = default;
    ~GpuParticleSystem();

    GpuParticleSystem(const GpuParticleSystem&) = delete;
    GpuParticleSystem& operator=(const GpuParticleSystem&) = delete;

    bool Initialize(const ParticleSystem& source);
    void Shutdown();
    bool BuildGrid();

    GLuint GetParticleBuffer() const;
    std::size_t GetParticleCount() const;

private:
    GLuint particleBuffers[2] = {};
    GLuint gridHeadBuffer = 0;
    GLuint nextParticleBuffer = 0;
    GLuint buildGridProgram = 0;

    std::size_t particleCount = 0;
    int activeBuffer = 0;
};