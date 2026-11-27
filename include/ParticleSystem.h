#ifndef AQUARIUM_PARTICLE_SYSTEM_H
#define AQUARIUM_PARTICLE_SYSTEM_H

class ParticleSystem {
public:
    enum { MaxParticles = 80 };

    ParticleSystem();
    int count() const;
    void getParticle(int index, float time,
                     float& x, float& y, float& z, float& radius) const;

private:
    struct Particle {
        float x;
        float z;
        float speed;
        float phase;
        float radius;
    };

    Particle particles[MaxParticles];
};

#endif
