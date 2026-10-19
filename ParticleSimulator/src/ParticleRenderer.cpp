#include "ParticleRenderer.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include <cstddef>
#include <vector>



namespace
{
    struct ParticleVertex
    {
        float x, y;
        float r, g, b;
    };

    std::string LoadShaderFile(const char* path)
    {
        std::ifstream file(path);

        if (!file)
        {
            std::cerr << "Failed to open shader: "
                << path << '\n';
            return {};
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str();
    }


    GLuint CompileShader(
        GLenum type,
        const std::string& source,
        const char* shaderName = "unknown")
    {
        GLuint shader = glCreateShader(type);

        const char* sourcePointer = source.c_str();
        glShaderSource(shader, 1, &sourcePointer, nullptr);
        glCompileShader(shader);

        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            char log[1024]{};
            glGetShaderInfoLog(
                shader,
                sizeof(log),
                nullptr,
                log
            );

            std::cerr << "Shader compilation failed ("
                << shaderName << "):\n"
                << log << '\n';

            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    GLuint CompileComputeShader(const std::string& path)
    {
        const std::string source = LoadShaderFile(path.c_str());

        if (source.empty())
            return 0;

        GLuint shader = CompileShader(
            GL_COMPUTE_SHADER,
            source
        );

        if (!shader)
        {
            std::cerr << "Failed to compile compute shader: "
                << path << '\n';
            return 0;
        }

        return shader;
    }

    GLuint CreateComputeProgram(const std::string& path)
    {
        GLuint computeShader = CompileComputeShader(path);

        if (!computeShader)
            return 0;

        GLuint program = glCreateProgram();

        glAttachShader(program, computeShader);
        glLinkProgram(program);

        GLint success = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success)
        {
            char log[1024]{};

            glGetProgramInfoLog(
                program,
                sizeof(log),
                nullptr,
                log
            );

            std::cerr << "Compute program linking failed for "
                << path << ":\n"
                << log << '\n';

            glDeleteProgram(program);
            program = 0;
        }

        glDeleteShader(computeShader);

        return program;
    }

    GLuint CreateShaderProgram()
    {
        const std::string vertexSource =
            LoadShaderFile("Shaders/particle.vert");

        const std::string fragmentSource =
            LoadShaderFile("Shaders/particle.frag");

        if (vertexSource.empty() || fragmentSource.empty())
            return 0;

        GLuint vertexShader =
            CompileShader(GL_VERTEX_SHADER, vertexSource,
                "particle.vert");

        if (!vertexShader)
            return 0;

        GLuint fragmentShader =
            CompileShader(GL_FRAGMENT_SHADER, fragmentSource,
                "particle.frag");

        if (!fragmentShader)
        {
            glDeleteShader(vertexShader);
            return 0;
        }

        GLuint program = glCreateProgram();

        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success)
        {
            char log[1024]{};
            glGetProgramInfoLog(program, sizeof(log), nullptr, log);

            std::cerr << "Shader linking failed:\n"
                << log << '\n';

            glDeleteProgram(program);
            program = 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    GLuint CreateGPUShaderProgram()
    {
        const std::string vertexSource =
            LoadShaderFile("Shaders/particle_gpu.vert");

        const std::string fragmentSource =
            LoadShaderFile("Shaders/particle_gpu.frag");

        if (vertexSource.empty() || fragmentSource.empty())
            return 0;

        GLuint vertexShader =
            CompileShader(GL_VERTEX_SHADER, vertexSource,
                "particle_gpu.vert");

        if (!vertexShader)
            return 0;

        GLuint fragmentShader =
            CompileShader(GL_FRAGMENT_SHADER, fragmentSource,
                "particle_gpu.frag");

        if (!fragmentShader)
        {
            glDeleteShader(vertexShader);
            return 0;
        }

        GLuint program = glCreateProgram();

        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success)
        {
            char log[1024]{};
            glGetProgramInfoLog(program, sizeof(log), nullptr, log);

            std::cerr << "GPU particle shader linking failed:\n"
                << log << '\n';

            glDeleteProgram(program);
            program = 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }
}

bool ParticleRenderer::Initialize()
{
    shaderProgram = CreateShaderProgram();

    if (!shaderProgram)
        return false;

    gpuParticleProgram = CreateGPUShaderProgram();

    if (!gpuParticleProgram)
    {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
        return false;
    }

    cameraPositionLocation =
        glGetUniformLocation(shaderProgram, "cameraPosition");

    zoomLocation =
        glGetUniformLocation(shaderProgram, "zoom");

    screenSizeLocation =
        glGetUniformLocation(shaderProgram, "screenSize");

    particleRadiusLocation =
        glGetUniformLocation(shaderProgram, "particleRadius");

    gpuCameraPositionLocation =
        glGetUniformLocation(gpuParticleProgram, "cameraPosition");

    gpuZoomLocation =
        glGetUniformLocation(gpuParticleProgram, "zoom");

    gpuScreenSizeLocation =
        glGetUniformLocation(gpuParticleProgram, "screenSize");

    gpuParticleRadiusLocation =
        glGetUniformLocation(gpuParticleProgram, "particleRadius");

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);

glVertexAttribPointer(
    0, 2, GL_FLOAT, GL_FALSE,
    sizeof(ParticleVertex),
    reinterpret_cast<void*>(offsetof(ParticleVertex, x))
);
glEnableVertexAttribArray(0);

glVertexAttribPointer(
    1, 3, GL_FLOAT, GL_FALSE,
    sizeof(ParticleVertex),
    reinterpret_cast<void*>(offsetof(ParticleVertex, r))
);
glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Enable gl_PointSize from the vertex shader.
    glEnable(GL_PROGRAM_POINT_SIZE);

    return true;
}

void ParticleRenderer::Render(
    const std::vector<Particle>& particles,
    float cameraX,
    float cameraY,
    float zoom,
    int screenWidth,
    int screenHeight,
    float particleRadius
)
{
    if (screenWidth <= 0 || screenHeight <= 0)
        return;

    // Pack particle positions into a GPU-friendly array.
    std::vector<ParticleVertex> vertices;
    vertices.reserve(particles.size());

    for (const Particle& p : particles)
    {
        vertices.push_back({
            p.x, p.y,
            p.r, p.g, p.b
            }
        );
    }

    glUseProgram(shaderProgram);

    glUniform2f(
        cameraPositionLocation,
        cameraX,
        cameraY
    );

    glUniform1f(zoomLocation, zoom);

    glUniform2f(
        screenSizeLocation,
        static_cast<float>(screenWidth),
        static_cast<float>(screenHeight)
    );

    glUniform1f(particleRadiusLocation, particleRadius);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            vertices.size() * sizeof(ParticleVertex)
            ),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );;

    glBindVertexArray(VAO);

    glDrawArrays(
        GL_POINTS,
        0,
        static_cast<GLsizei>(particles.size())
    );

    glBindVertexArray(0);
}


void ParticleRenderer::RenderGPU(
    const GpuParticleSystem& particles,
    float cameraX,
    float cameraY,
    float zoom,
    int screenWidth,
    int screenHeight,
    float particleRadius
)
{
    if (screenWidth <= 0 || screenHeight <= 0)
        return;

    const GLuint particleBuffer =
        particles.GetParticleBuffer();

    const std::size_t particleCount =
        particles.GetParticleCount();

    if (particleBuffer == 0 || particleCount == 0)
        return;

    glUseProgram(gpuParticleProgram);

    glUniform2f(
        gpuCameraPositionLocation,
        cameraX,
        cameraY
    );

    glUniform1f(gpuZoomLocation, zoom);

    glUniform2f(
        gpuScreenSizeLocation,
        static_cast<float>(screenWidth),
        static_cast<float>(screenHeight)
    );

    glUniform1f(
        gpuParticleRadiusLocation,
        particleRadius
    );

    // Binding 0 matches particle_gpu.vert.
    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        0,
        particleBuffer
    );

    glBindVertexArray(VAO);

    glDrawArrays(
        GL_POINTS,
        0,
        static_cast<GLsizei>(particleCount)
    );

    glBindVertexArray(0);
    glUseProgram(0);
}

void ParticleRenderer::Shutdown()
{
    if (VBO)
        glDeleteBuffers(1, &VBO);

    if (VAO)
        glDeleteVertexArrays(1, &VAO);

    if (shaderProgram)
        glDeleteProgram(shaderProgram);

    if (gpuParticleProgram)
        glDeleteProgram(gpuParticleProgram);

    VBO = 0;
    VAO = 0;
    shaderProgram = 0;
    gpuParticleProgram = 0;
}