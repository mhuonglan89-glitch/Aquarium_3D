#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdlib>
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
    float Speed;        // Vận tốc nổi thẳng đứng hướng lên mặt nước (units/giây)
    float Radius;       // Kích thước bán kính hạt bọt khí
};

class ParticleSystem {
public:
    std::vector<BubbleParticle> particles; // Danh sách 100 hạt bọt khí
    unsigned int VAO, VBO;                 // Bộ đệm hình học của 1 hạt
    int particleCount;                     // Tổng số lượng hạt khởi tạo

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: KHỞI TẠO HỆ HẠT VÀ TẠO BỘ ĐỆM ĐỈNH HÌNH VUÔNG BILLBOARD]
    // ------------------------------------------------------------------------------------------
    ParticleSystem(int count = 100) : particleCount(count) {
        // Khởi tạo ngẫu nhiên vị trí ban đầu cho mảng hạt
        for (int i = 0; i < count; ++i) {
            BubbleParticle b;
            resetBubble(b, true); // true = Rải ngẫu nhiên độ cao Y ban đầu
            particles.push_back(b);
        }

        // Tạo mảng đỉnh 1 hình vuông (Quad) gồm 2 tam giác để áp dụng kỹ thuật Billboard
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

        // aPos: Thuộc tính đỉnh vị trí X, Y, Z (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: CẬP NHẬT VẬN TỐC, TÁI SINH VÀ VẼ HỆ HẠT (UPDATE & RENDER)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Cập nhật tọa độ nổi Y và vẽ từng hạt bọt khí với độ trong suốt
     */
    void UpdateAndRender(float dt, glm::vec3 camPos, Shader& shader) {
        // Bật chế độ hòa trộn màu Alpha Blending (Chương CG.6)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(VAO);

        // Màu trắng ánh xanh lơ lấp lánh của bọt khí
        shader.setVec3("objectColor", glm::vec3(0.75f, 0.92f, 1.0f));
        shader.setFloat("alphaVal", 0.45f); // Độ mờ trong suốt 45%

        for (auto& b : particles) {
            // 1. Cập nhật vị trí nổi lên theo thời gian: Y = Y + Speed * deltaTime
            b.Position.y += b.Speed * dt;

            // 2. Cơ chế tái sinh hạt (Recycling): Khi bọt chạm mặt nước (Y > 250), đưa lại về đáy cát
            if (b.Position.y > 250.0f) {
                resetBubble(b, false); // false = Đặt chính xác tại đáy cát Y = -150
            }

            // 3. Ma trận mô hình (Model Matrix) dịch chuyển hạt tới vị trí và co dãn theo bán kính
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, b.Position);
            model = glm::scale(model, glm::vec3(b.Radius));
            shader.setMat4("model", model);

            // 4. Vẽ 2 tam giác tạo nên hình vuông bọt khí
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glBindVertexArray(0);
    }

private:
    // ------------------------------------------------------------------------------------------
    // [CỤM 4: THUẬT TOÁN TÁI SINH HẠT NGẪU NHIÊN Ở ĐÁY BIỂN (RECYCLING ALGORITHM)]
    // ------------------------------------------------------------------------------------------
    void resetBubble(BubbleParticle& b, bool randomY) {
        // Rải ngẫu nhiên hạt dọc theo khu vực có rạn san hô
        b.Position.x = -320.0f + static_cast<float>(rand() % 640);
        b.Position.z = -650.0f + static_cast<float>(rand() % 1300);
        b.Position.y = randomY ? (-150.0f + static_cast<float>(rand() % 400)) : -150.0f;
        
        // Vận tốc nổi ngẫu nhiên từ 35 đến 70 units/giây
        b.Speed = 35.0f + static_cast<float>(rand() % 35);
        
        // Bán kính ngẫu nhiên tạo độ to nhỏ sinh động
        b.Radius = 1.2f + static_cast<float>(rand() % 3) * 0.8f;
    }
};
