#include "ParticleSystem.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <random>

constexpr float WORLD_LEFT = -500.0f;
constexpr float WORLD_RIGHT = 500.0f;
constexpr float WORLD_BOTTOM = -300.0f;
constexpr float WORLD_TOP = 300.0f;

static std::uint64_t MakeCellKey(int x, int y)
{
    const std::uint64_t ux = static_cast<std::uint32_t>(x);
    const std::uint64_t uy = static_cast<std::uint32_t>(y);

    return (ux << 32) | uy;
}

ParticleSystem::ParticleSystem(int count)
    : rng(std::random_device{}()),
    positionDist(-250.0f, 250.0f),
    velocityDist(-100.0f, 100.0f)
{
    particles.resize(count);
    Reset();
}

void ParticleSystem::InitializeParticle(Particle& p)
{
    p.x = positionDist(rng);
    p.y = positionDist(rng);

    p.vx = velocityDist(rng);
    p.vy = velocityDist(rng);

    p.r = colorDist(rng);
    p.g = colorDist(rng);
    p.b = colorDist(rng);
}

void ParticleSystem::Reset()
{
    for (Particle& p : particles)
    {
        InitializeParticle(p);
    }
}

void ParticleSystem::Update(float deltaTime)
{
    // 1. Apply gravity and move particles.
    for (Particle& p : particles)
    {
        p.vy += gravity * deltaTime;

        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
    }

    // 2. Resolve collisions between particles.
    ResolveParticleCollisions();

    // 3. Resolve collisions with the world boundaries.
    for (Particle& p : particles)
    {
        if (p.x < WORLD_LEFT + particleRadius)
        {
            p.x = WORLD_LEFT + particleRadius;
            p.vx = std::abs(p.vx) * restitution;
        }
        else if (p.x > WORLD_RIGHT - particleRadius)
        {
            p.x = WORLD_RIGHT - particleRadius;
            p.vx = -std::abs(p.vx) * restitution;
        }

        if (p.y < WORLD_BOTTOM + particleRadius)
        {
            p.y = WORLD_BOTTOM + particleRadius;
            p.vy = std::abs(p.vy) * restitution;
        }
        else if (p.y > WORLD_TOP - particleRadius)
        {
            p.y = WORLD_TOP - particleRadius;
            p.vy = -std::abs(p.vy) * restitution;
        }
    }
}


void ParticleSystem::ResolveParticleCollisions()
{
    const float diameter = particleRadius * 2.0f;
    const float minDistanceSquared = diameter * diameter;

    // Two passes help resolve overlaps caused by earlier collisions.
    constexpr int COLLISION_PASSES = 2;

    using Grid = std::unordered_map<
        std::uint64_t,
        std::vector<std::size_t>
    >;

    for (int pass = 0; pass < COLLISION_PASSES; ++pass)
    {
        Grid grid;

        // 1. Put each particle into its grid cell.
        for (std::size_t i = 0; i < particles.size(); ++i)
        {
            const Particle& p = particles[i];

            const int cellX = static_cast<int>(
                std::floor(p.x / diameter)
                );

            const int cellY = static_cast<int>(
                std::floor(p.y / diameter)
                );

            grid[MakeCellKey(cellX, cellY)].push_back(i);
        }

        // 2. Check particles in the current cell and its neighbors.
        for (std::size_t i = 0; i < particles.size(); ++i)
        {
            Particle& a = particles[i];

            const int cellX = static_cast<int>(
                std::floor(a.x / diameter)
                );

            const int cellY = static_cast<int>(
                std::floor(a.y / diameter)
                );

            for (int offsetY = -1; offsetY <= 1; ++offsetY)
            {
                for (int offsetX = -1; offsetX <= 1; ++offsetX)
                {
                    const auto it = grid.find(
                        MakeCellKey(
                            cellX + offsetX,
                            cellY + offsetY
                        )
                    );

                    if (it == grid.end())
                        continue;

                    for (std::size_t j : it->second)
                    {
                        // Process each pair only once.
                        if (j <= i)
                            continue;

                        Particle& b = particles[j];

                        const float dx = b.x - a.x;
                        const float dy = b.y - a.y;

                        const float distanceSquared =
                            dx * dx + dy * dy;

                        if (distanceSquared >= minDistanceSquared)
                            continue;

                        float distance;
                        float nx;
                        float ny;

                        if (distanceSquared < 0.000001f)
                        {
                            distance = 0.0f;
                            nx = 1.0f;
                            ny = 0.0f;
                        }
                        else
                        {
                            distance = std::sqrt(distanceSquared);
                            nx = dx / distance;
                            ny = dy / distance;
                        }

                        // 3. Separate the overlapping particles.
                        const float overlap = diameter - distance;
                        const float correction = overlap * 0.5f;

                        a.x -= nx * correction;
                        a.y -= ny * correction;

                        b.x += nx * correction;
                        b.y += ny * correction;

                        // 4. Apply an impulse if they are approaching.
                        const float relativeVx = b.vx - a.vx;
                        const float relativeVy = b.vy - a.vy;

                        const float velocityAlongNormal =
                            relativeVx * nx + relativeVy * ny;

                        if (velocityAlongNormal >= 0.0f)
                            continue;

                        const float impulse =
                            -(1.0f + restitution)
                            * velocityAlongNormal * 0.5f;

                        a.vx -= impulse * nx;
                        a.vy -= impulse * ny;

                        b.vx += impulse * nx;
                        b.vy += impulse * ny;
                    }
                }
            }
        }
    }
}

const std::vector<Particle>& ParticleSystem::GetParticles() const
{
    return particles;
}

void ParticleSystem::SetGravity(float value)
{
    gravity = value;
}

float ParticleSystem::GetGravity() const
{
    return gravity;
}

void ParticleSystem::SetParticleRadius(float value)
{
    particleRadius = std::max(value, 0.5f);
}

float ParticleSystem::GetParticleRadius() const
{
    return particleRadius;
}