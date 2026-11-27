#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>
#include "Shader.h"

/**
 * ==============================================================================================
 * 📌 FILE: include/ParticleSystem.h
 * 👤 PHỤ TRÁCH: BẠN B (Frontend / Kỹ sư Hiệu Ứng Hệ Hạt & Độ Trong Suốt)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.6: KỸ THUẬT ĐỒ HỌA NÂNG CAO (Advanced Graphics Techniques)
 *      -> Hệ hạt bọt khí (Particle System): Vòng đời, tái sinh hạt (Recycling)
 *      -> Hiệu ứng mặt trời chiếu qua hạt trong suốt (Alpha Blending: glBlendFunc)
 *      -> Kỹ thuật Billboard ma trận (Mặt hạt luôn quay trực diện về phía Camera)
 * ==============================================================================================
 */

// ----------------------------------------------------------------------------------------------
// [CỤM 1: CẤU TRÚC DỮ LIỆU CỦA 1 HẠT BỌT KHÍ ĐẠI DƯƠNG]
// ----------------------------------------------------------------------------------------------
struct BubbleParticle {
    glm::vec3 Position; // Tọa độ 3D hiện tại của hạt bọt khí
    float BaseX;        // Tọa độ X gốc để dao động lắc lư hình sin khi nổi
    float Speed;        // Vận tốc nổi thẳng đứng hướng lên mặt nước (units/giây)
    float Radius;       // Kích thước bán kính hạt bọt khí
    float Phase;        // Pha dao động ngang riêng của từng hạt
};

class ParticleSystem {
public:
    std::vector<BubbleParticle> particles; // Danh sách các hạt bọt khí
    unsigned int VAO = 0, VBO = 0;         // Bộ đệm hình học của 1 hạt
    int particleCount;                     // Tổng số lượng hạt khởi tạo

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: KHỞI TẠO HỆ HẠT VÀ TẠO BỘ ĐỆM ĐỈNH HÌNH BÁT GIÁC BILLBOARD]
    // ------------------------------------------------------------------------------------------
    ParticleSystem(int count = 240) : particleCount(count) {
        for (int i = 0; i < count; ++i) {
            BubbleParticle b;
            resetBubble(b, true); // true = Rải ngẫu nhiên độ cao Y ban đầu từ 5 -> 480
            particles.push_back(b);
        }

        // Tạo hình đa giác tròn 8 cạnh (Octagon Fan) để hạt bọt khí tròn mượt tự nhiên hơn hình vuông
        std::vector<float> circleVerts;
        const int segs = 10;
        for (int i = 0; i < segs; ++i) {
            float a1 = 2.0f * 3.14159265f * (float)i / segs;
            float a2 = 2.0f * 3.14159265f * (float)(i + 1) / segs;
            // Tâm (0,0,0), pháp tuyến hướng +Z (0,0,1)
            circleVerts.insert(circleVerts.end(), {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
            circleVerts.insert(circleVerts.end(), {cosf(a1), sinf(a1), 0.0f, 0.0f, 0.0f, 1.0f});
            circleVerts.insert(circleVerts.end(), {cosf(a2), sinf(a2), 0.0f, 0.0f, 0.0f, 1.0f});
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, circleVerts.size() * sizeof(float), circleVerts.data(), GL_STATIC_DRAW);

        // aPos (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // aNormal (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    ~ParticleSystem() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: CẬP NHẬT VẬN TỐC, TÁI SINH VÀ VẼ HỆ HẠT BILLBOARD (UPDATE & RENDER)]
    // ------------------------------------------------------------------------------------------
    void UpdateAndRender(float dt, float totalTime, glm::vec3 camPos, Shader& shader) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Không ghi đè Z-Buffer để không che khuất cá phía sau

        shader.setBool("useTexture", false);
        shader.setBool("isFish", false);
        shader.setFloat("causticStrength", 0.0f);
        shader.setVec3("objectColor", glm::vec3(0.82f, 0.96f, 1.0f));
        shader.setFloat("alphaVal", 0.42f);

        glBindVertexArray(VAO);

        for (auto& b : particles) {
            // 1. Nổi lên theo trục Y + lắc lư ngang theo sóng sin
            b.Position.y += b.Speed * dt;
            b.Position.x = b.BaseX + sinf(totalTime * 2.2f + b.Phase) * 6.0f;

            // 2. Tái sinh khi chạm mặt nước (Y > 480) -> đưa về sát đáy cát/rạn san hô (Y = 4)
            if (b.Position.y > 480.0f) {
                resetBubble(b, false);
            }

            // 3. Kỹ thuật Billboard: Xoay mặt bọt khí luôn hướng về phía Camera
            glm::vec3 toCam = camPos - b.Position;
            float yaw = atan2f(toCam.x, toCam.z);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, b.Position);
            model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(b.Radius));
            shader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 30);
        }

        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
    }

private:
    // ------------------------------------------------------------------------------------------
    // [CỤM 4: THUẬT TOÁN TÁI SINH HẠT NGẪU NHIÊN NGOÀI HẦM KÍNH (RECYCLING ALGORITHM)]
    // ------------------------------------------------------------------------------------------
    void resetBubble(BubbleParticle& b, bool randomY) {
        // Rải bọt khí tại 4 khoảng biển mở xung quanh Đại Sảnh & các nhánh mê cung (tránh bay vào trong hầm kính)
        float sx = (rand() % 2 == 0) ? 1.0f : -1.0f;
        float sz = (rand() % 2 == 0) ? 1.0f : -1.0f;
        b.BaseX = sx * (175.0f + static_cast<float>(rand() % 360));
        b.Position.x = b.BaseX;
        b.Position.z = sz * (175.0f + static_cast<float>(rand() % 420));
        b.Position.y = randomY ? (8.0f + static_cast<float>(rand() % 470)) : 8.0f;

        b.Speed = 28.0f + static_cast<float>(rand() % 38);
        b.Radius = 1.6f + static_cast<float>(rand() % 4) * 0.75f;
        b.Phase = static_cast<float>(rand() % 628) * 0.01f;
    }
};
