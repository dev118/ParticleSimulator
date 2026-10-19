#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <algorithm>

#include <iostream>

#include "ParticleSystem.h"
#include "ParticleRenderer.h"
#include "Camera2D.h"
#include "GpuParticleSystem.h"

#ifdef _WIN32

extern "C" {
    __declspec(dllexport)
        unsigned long NvOptimusEnablement = 0x00000001;

    __declspec(dllexport)
        int AmdPowerXpressRequestHighPerformance = 1;
}

#endif


float scrollDelta = 0.0f;

void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    scrollDelta += static_cast<float>(yOffset);
}


int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Request an OpenGL 4.3 Core context.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(
        1280, 720,
        "Particle Simulator",
        nullptr, nullptr
    );

    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetScrollCallback(window, ScrollCallback);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // After gladLoadGL succeeds:
    std::cout << "OpenGL vendor: "
        << glGetString(GL_VENDOR) << '\n';

    std::cout << "OpenGL renderer: "
        << glGetString(GL_RENDERER) << '\n';

    std::cout << "OpenGL version: "
        << glGetString(GL_VERSION) << '\n';

    ParticleRenderer particleRenderer;

    if (!particleRenderer.Initialize())
    {
        std::cerr << "Failed to initialize particle renderer.\n";

        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");




    constexpr int PARTICLE_COUNT = 1000;

    float particleRadius = 4.0f;

    ParticleSystem particleSystem(PARTICLE_COUNT);
    particleSystem.SetParticleRadius(particleRadius);

    GpuParticleSystem gpuParticleSystem;

    if (!gpuParticleSystem.Initialize(particleSystem))
    {
        std::cerr << "Failed to initialize GPU particle system.\n";
        return -1;
    }

   /* gpuParticleSystem.SetComputePrograms(
        buildGridProgram,
        solveCollisionsProgram);*/

    // Main rendering loop.
    bool paused = false;

    float previousTime = static_cast<float>(glfwGetTime());

   

    Camera2D camera;

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - previousTime;
        deltaTime = std::min(deltaTime, 0.033f);
        previousTime = currentTime;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Particle Simulator");

        float gravity = particleSystem.GetGravity();

        if (ImGui::SliderFloat("Gravity", &gravity, -300.0f, 300.0f))
        {
            particleSystem.SetGravity(gravity);
        }

        ImGui::Checkbox("Pause simulation", &paused);

        if (ImGui::Button("Reset particles"))
        {
            particleSystem.Reset();

            if (!gpuParticleSystem.Initialize(particleSystem))
            {
                std::cerr << "Failed to reset GPU particle system.\n";
            }
        }

        ImGui::Text(
            "Particles: %d",
            static_cast<int>(particleSystem.GetParticles().size())
        );

        if (ImGui::SliderFloat(
            "Particle radius",
            &particleRadius,
            1.0f,
            12.0f))
        {
            particleSystem.SetParticleRadius(particleRadius);
        }

        ImGui::End();

        // Simulate particles on the CPU.



        const float cameraMove = camera.moveSpeed * deltaTime / camera.zoom;

        // Move camera with WASD.
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.x -= cameraMove;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.x += cameraMove;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.y += cameraMove;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.y -= cameraMove;

        // Zoom using the mouse wheel.
        if (scrollDelta != 0.0f)
        {
            camera.zoom *= std::pow(1.1f, scrollDelta);

            camera.zoom = std::clamp(camera.zoom, 0.1f, 10.0f);

            scrollDelta = 0.0f;
        }

        // Reset camera with R.
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            camera.Reset();
        }


        // Update particle physics.
        if (!paused)
        {
            //particleSystem.Update(deltaTime);
            gpuParticleSystem.Simulate(deltaTime,gravity, particleRadius, 0.2);
        }

        // Get framebuffer dimensions for correct window scaling.
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        if (width > 0 && height > 0)
        {
            glViewport(0, 0, width, height);

            glClearColor(0.02f, 0.02f, 0.04f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            /*particleRenderer.Render(
                particleSystem.GetParticles(),
                camera.x,
                camera.y,
                camera.zoom,
                width,
                height,
                particleRadius
            );*/

            particleRenderer.RenderGPU(
                gpuParticleSystem,
                camera.x,
                camera.y,
                camera.zoom,
                width,
                height,
                particleRadius);
        }

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(
            ImGui::GetDrawData()
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up OpenGL resources.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    particleRenderer.Shutdown();
    gpuParticleSystem.Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}