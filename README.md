# Particle Simulator

A real-time particle simulation built with **C++**, **OpenGL**, and
**Dear ImGui**.

The project explores particle motion, collision handling, rendering, and
performance-oriented simulation techniques. It is a personal
graphics-programming project and is being developed incrementally.

## Screenshot

![Particle Simulator running](docs/images/particle-simulator.png)

> To add your screenshot, create `docs/images/`, save the screenshot as
> `particle-simulator.png`, and commit it to the repository. The image
> above will then appear directly in the GitHub README.

## Features

-   Real-time particle rendering with OpenGL
-   Interactive interface built with Dear ImGui
-   2D camera controls
-   CPU particle simulation and collision handling
-   Spatial-grid-based broad-phase collision optimization
-   GPU particle-system and compute-shader experiments in progress

## Built With

-   **C++**
-   **OpenGL** --- rendering
-   **GLFW** --- window and input handling
-   **GLAD** --- OpenGL function loading
-   **Dear ImGui** --- immediate-mode user interface
-   **GLSL** --- vertex, fragment, and compute shaders

## Project Structure

The exact layout may evolve as the project is organized further. The
main application code currently includes:

-   `Camera2D` --- camera movement and zoom
-   `ParticleSystem` --- CPU-side particle simulation
-   `ParticleRenderer` --- particle rendering
-   `GpuParticleSystem` --- GPU simulation experiments
-   `Shaders` --- GLSL shader files

## Getting Started

### Requirements

-   Windows
-   Visual Studio with the **Desktop development with C++** workload
-   A graphics card and driver supporting the OpenGL version used by the
    project

The project currently uses a Visual Studio project setup. Build
instructions may change as the build system is updated.

### Build and Run

1.  Clone or download this repository.
2.  Open `ParticleSimulator.sln` in Visual Studio.
3.  Select the desired configuration, such as **Debug** or **Release**.
4.  Build the solution.
5.  Run the `ParticleSimulator` project.

If the solution or project file names change, follow the files present
in the repository.

## Development Notes

This project is actively evolving. The GPU compute path is experimental
and should not be considered a complete GPU-based collision solver yet.
Performance and implementation details may change as the simulation
develops.

## Acknowledgements

-   [Dear ImGui](https://github.com/ocornut/imgui)
-   [GLFW](https://www.glfw.org/)
-   [GLAD](https://glad.dav1d.de/)
-   [OpenGL](https://www.opengl.org/)

## License

Check the repository for the applicable license and the licenses of its
third-party dependencies.
