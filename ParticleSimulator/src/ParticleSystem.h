
#pragma once

#include <vector>
#include <random>

struct Particle
{
    float x;
    float y;

    float vx;
    float vy;

    float r, g, b;
};

class ParticleSystem
{
public:
    explicit ParticleSystem(int count);

    void Update(float deltaTime);
    void Reset();

    const std::vector<Particle>& GetParticles() const;

    void SetGravity(float value);
    float GetGravity() const;

    void SetParticleRadius(float value);
    float GetParticleRadius() const;

private:
    std::vector<Particle> particles;

    std::mt19937 rng;
    std::uniform_real_distribution<float> positionDist;
    std::uniform_real_distribution<float> velocityDist;
    std::uniform_real_distribution<float> colorDist;

    float gravity = -0.8f;
    float restitution = 0.85f;
    float particleRadius = 4.0f;

    void InitializeParticle(Particle& particle);
    void ResolveParticleCollisions();
};