#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdlib>
#include "Shader.h"

struct BubbleParticle {
    glm::vec3 Position;
    float Speed;
    float Radius;
};

/**
 * @file ParticleSystem.h
 * @brief Hệ hạt bọt khí nổi lên từ đáy biển (Chương CG.6 Particle & Blending)
 * @author Bạn B (Frontend / Effects) - Môn Đồ Họa Máy Tính
 */
class ParticleSystem {
public:
    std::vector<BubbleParticle> particles;
    unsigned int VAO, VBO;
    int particleCount;

    ParticleSystem(int count = 80) : particleCount(count) {
        for (int i = 0; i < count; ++i) {
            BubbleParticle b;
            resetBubble(b, true);
            particles.push_back(b);
        }

        // Tạo mảng đỉnh hình cầu nhỏ hoặc điểm point
        std::vector<float> quadVerts = {
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(float), quadVerts.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void UpdateAndRender(float dt, glm::vec3 camPos, Shader& shader) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(VAO);

        shader.setVec3("objectColor", glm::vec3(0.7f, 0.9f, 1.0f));
        shader.setFloat("alphaVal", 0.45f); // Bọt khí bán trong suốt

        for (auto& b : particles) {
            b.Position.y += b.Speed * dt;
            if (b.Position.y > 250.0f) {
                resetBubble(b, false);
            }

            // Billboard ma trận để mặt bọt khí luôn quay về camera
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, b.Position);
            model = glm::scale(model, glm::vec3(b.Radius));
            shader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glBindVertexArray(0);
    }

private:
    void resetBubble(BubbleParticle& b, bool randomY) {
        b.Position.x = -300.0f + static_cast<float>(rand() % 600);
        b.Position.z = -600.0f + static_cast<float>(rand() % 1200);
        b.Position.y = randomY ? (-150.0f + static_cast<float>(rand() % 400)) : -150.0f;
        b.Speed = 35.0f + static_cast<float>(rand() % 35);
        b.Radius = 1.2f + static_cast<float>(rand() % 3) * 0.8f;
    }
};
