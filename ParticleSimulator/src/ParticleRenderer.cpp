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

    GLuint CompileShader( GLenum type, const std::string& source){
        GLuint shader = glCreateShader(type);

        const char* sourcePointer = source.c_str();
        glShaderSource(shader, 1, &sourcePointer, nullptr);
        glCompileShader(shader);

        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            char log[1024]{};
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);

            std::cerr << "Shader compilation failed:\n" << log << '\n';

            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    GLuint CreateShaderProgram()
    {
        const std::string vertexSource =
            LoadShaderFile("src/Shaders/particle.vert");

        const std::string fragmentSource =
            LoadShaderFile("src/Shaders/particle.frag");

        if (vertexSource.empty() || fragmentSource.empty())
            return 0;

        GLuint vertexShader =
            CompileShader(GL_VERTEX_SHADER, vertexSource);

        if (!vertexShader)
            return 0;

        GLuint fragmentShader =
            CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

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
}

bool ParticleRenderer::Initialize()
{
    shaderProgram = CreateShaderProgram();

    if (!shaderProgram)
        return false;

    cameraPositionLocation =
        glGetUniformLocation(shaderProgram, "cameraPosition");

    zoomLocation =
        glGetUniformLocation(shaderProgram, "zoom");

    screenSizeLocation =
        glGetUniformLocation(shaderProgram, "screenSize");

    particleRadiusLocation =
        glGetUniformLocation(shaderProgram, "particleRadius");

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

void ParticleRenderer::Shutdown()
{
    if (VBO)
        glDeleteBuffers(1, &VBO);

    if (VAO)
        glDeleteVertexArrays(1, &VAO);

    if (shaderProgram)
        glDeleteProgram(shaderProgram);

    VBO = 0;
    VAO = 0;
    shaderProgram = 0;
}