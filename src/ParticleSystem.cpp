#include "ParticleSystem.h"
#include <cmath>

ParticleSystem::ParticleSystem() {
    const float sourceX[3] = {-3.65f, 3.55f, 2.95f};
    const float sourceZ[3] = {-1.90f, -1.20f, 2.00f};
    int i;
    for (i = 0; i < MaxParticles; ++i) {
        int emitter = i % 3;
        float spread = 0.16f + std::fmod(i * 0.071f, 0.32f);
        particles[i].x = sourceX[emitter] + spread * std::sin(i * 2.17f);
        particles[i].z = sourceZ[emitter] + spread * std::cos(i * 1.83f);
        particles[i].speed = 0.52f + std::fmod(i * 0.17f, 0.5f);
        particles[i].phase = std::fmod(i * 2.399963f, 4.3f);
        particles[i].radius = 0.035f + std::fmod(i * 0.013f, 0.03f);
    }
}

int ParticleSystem::count() const {
    return MaxParticles;
}

void ParticleSystem::getParticle(int index, float time,
                                 float& x, float& y, float& z,
                                 float& radius) const {
    if (index < 0 || index >= MaxParticles) {
        x = 0.0f;
        y = -2.0f;
        z = 0.0f;
        radius = 0.04f;
        return;
    }

    const Particle& p = particles[index];
    float travel = std::fmod(p.phase + time * p.speed, 4.3f);
    if (travel < 0.0f) {
        travel += 4.3f;
    }

    x = p.x + 0.12f * std::sin(time * 1.6f + p.phase);
    y = -2.12f + travel;
    z = p.z + 0.08f * std::cos(time * 1.25f + p.phase);
    radius = p.radius;
}
