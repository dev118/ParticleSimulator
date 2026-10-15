#include "GpuParticleSystem.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    // World bounds: -500 to 500 horizontally,
    // -300 to 300 vertically.
    constexpr float WORLD_LEFT = -500.0f;
    constexpr float WORLD_BOTTOM = -300.0f;

    constexpr int GRID_COLUMNS = 125;
    constexpr int GRID_ROWS = 75;
    constexpr int GRID_CELL_COUNT =
        GRID_COLUMNS * GRID_ROWS;

    constexpr float GRID_CELL_SIZE = 8.0f;
}

GpuParticleSystem::~GpuParticleSystem()
{
    Shutdown();
}

bool GpuParticleSystem::Initialize( const ParticleSystem& source)
{
    Shutdown();

    const auto& particles = source.GetParticles();
    particleCount = particles.size();

    if (particleCount == 0)
        return true;

    if (particleCount >
        static_cast<std::size_t>(
            std::numeric_limits<GLsizeiptr>::max() /
            sizeof(ParticleGPU)))
    {
        std::cerr << "Too many particles for GPU buffer.\n";
        particleCount = 0;
        return false;
    }

    std::vector<ParticleGPU> initialParticles;
    initialParticles.reserve(particleCount);

    for (const Particle& p : particles)
    {
        initialParticles.push_back({
            p.x, p.y, p.vx, p.vy
            });
    }

    const GLsizeiptr particleBytes =
        static_cast<GLsizeiptr>(
            particleCount * sizeof(ParticleGPU));

    // Create two particle buffers for ping-pong processing.
    glGenBuffers(2, particleBuffers);

    for (int i = 0; i < 2; ++i)
    {
        glBindBuffer(
            GL_SHADER_STORAGE_BUFFER,
            particleBuffers[i]);

        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            particleBytes,
            i == 0 ? initialParticles.data() : nullptr,
            GL_DYNAMIC_COPY);
    }

    // Each grid cell stores the index of its first particle.
    glGenBuffers(1, &gridHeadBuffer);

    glBindBuffer(
        GL_SHADER_STORAGE_BUFFER,
        gridHeadBuffer);

    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        GRID_CELL_COUNT * sizeof(GLint),
        nullptr,
        GL_DYNAMIC_COPY);

    // Each particle stores the next particle in its cell.
    glGenBuffers(1, &nextParticleBuffer);

    glBindBuffer(
        GL_SHADER_STORAGE_BUFFER,
        nextParticleBuffer);

    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        static_cast<GLsizeiptr>(
            particleCount * sizeof(GLint)),
        nullptr,
        GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    activeBuffer = 0;

    std::cout << "GPU particle buffers initialized: "
        << particleCount << '\n';

    return true;
}

void GpuParticleSystem::Shutdown()
{
    glDeleteBuffers(2, particleBuffers);
    glDeleteBuffers(1, &gridHeadBuffer);
    glDeleteBuffers(1, &nextParticleBuffer);

    particleBuffers[0] = 0;
    particleBuffers[1] = 0;
    gridHeadBuffer = 0;
    nextParticleBuffer = 0;

    particleCount = 0;
    activeBuffer = 0;
}


bool GpuParticleSystem::BuildGrid()
{
    if (particleCount == 0 || buildGridProgram == 0)
        return false;

    // Clear every grid cell to -1 (empty).
    std::vector<GLint> emptyHeads(GRID_CELL_COUNT, -1);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridHeadBuffer);
    glBufferSubData(
        GL_SHADER_STORAGE_BUFFER,
        0,
        emptyHeads.size() * sizeof(GLint),
        emptyHeads.data()
    );

    // Bind the particle and grid buffers to the bindings used by the shader.
    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER, 0, particleBuffers[activeBuffer]
    );
    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER, 1, gridHeadBuffer
    );
    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER, 2, nextParticleBuffer
    );

    glUseProgram(buildGridProgram);

    glUniform1ui(
        glGetUniformLocation(buildGridProgram, "particleCount"),
        static_cast<GLuint>(particleCount)
    );
    glUniform1i(
        glGetUniformLocation(buildGridProgram, "gridColumns"),
        GRID_COLUMNS
    );
    glUniform1i(
        glGetUniformLocation(buildGridProgram, "gridRows"),
        GRID_ROWS
    );
    glUniform1f(
        glGetUniformLocation(buildGridProgram, "cellSize"),
        GRID_CELL_SIZE
    );
    glUniform2f(
        glGetUniformLocation(buildGridProgram, "worldMinimum"),
        WORLD_LEFT,
        WORLD_BOTTOM
    );

    // One shader invocation per particle, with 256 invocations per workgroup.
    GLuint workGroups =
        static_cast<GLuint>((particleCount + 255) / 256);

    glDispatchCompute(workGroups, 1, 1);

    // Ensure compute-shader writes are visible to later GPU operations.
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return true;
}

GLuint GpuParticleSystem::GetParticleBuffer() const
{
    return particleBuffers[activeBuffer];
}

std::size_t GpuParticleSystem::GetParticleCount() const
{
    return particleCount;
}