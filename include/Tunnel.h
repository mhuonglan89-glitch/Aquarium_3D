#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

/**
 * @file Tunnel.h
 * @brief Sinh lưới vòm kính hình trụ & sàn hầm bằng toán học tham số
 * @author Bạn A (Backend / Math & Geometry) - Môn Đồ Họa Máy Tính (Chương CG.5)
 */
class Tunnel {
public:
    unsigned int VAO_Glass, VBO_Glass;
    unsigned int VAO_Floor, VBO_Floor;
    int glassVertexCount;
    int floorVertexCount;

    float Radius;
    float Length;

    Tunnel(float radius = 160.0f, float length = 1500.0f, int segments = 32, int stepsZ = 30)
        : Radius(radius), Length(length) {
        
        generateGlassArch(radius, length, segments, stepsZ);
        generateFloor(radius * 0.7f, length, stepsZ);
    }

    ~Tunnel() {
        glDeleteVertexArrays(1, &VAO_Glass);
        glDeleteBuffers(1, &VBO_Glass);
        glDeleteVertexArrays(1, &VAO_Floor);
        glDeleteBuffers(1, &VBO_Floor);
    }

    // Vẽ toàn bộ hầm thủy cung
    void Draw(class Shader& shader) {
        // 1. Vẽ sàn hầm gạch tối màu
        glBindVertexArray(VAO_Floor);
        glDrawArrays(GL_TRIANGLES, 0, floorVertexCount);

        // 2. Vẽ vòm kính thủy cung (Bật chế độ trong suốt)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(VAO_Glass);
        glDrawArrays(GL_TRIANGLES, 0, glassVertexCount);
    }

private:
    // Sinh lưới vòm kính nửa hình trụ: x = R*cos(theta), y = R*sin(theta), z chạy dọc chiều dài
    void generateGlassArch(float r, float l, int segs, int stepsZ) {
        std::vector<float> verts;
        float stepZ = l / stepsZ;

        for (int i = 0; i < segs; ++i) {
            float t1 = 3.14159265f * (float)i / segs;
            float t2 = 3.14159265f * (float)(i + 1) / segs;

            for (int j = 0; j < stepsZ; ++j) {
                float z1 = -l / 2.0f + j * stepZ;
                float z2 = z1 + stepZ;

                glm::vec3 v1(r * cos(t1), r * sin(t1), z1);
                glm::vec3 v2(r * cos(t2), r * sin(t2), z1);
                glm::vec3 v3(r * cos(t1), r * sin(t1), z2);
                glm::vec3 v4(r * cos(t2), r * sin(t2), z2);

                // Tam giác 1
                pushVertex(verts, v1, (float)i/segs, (float)j/stepsZ);
                pushVertex(verts, v2, (float)(i+1)/segs, (float)j/stepsZ);
                pushVertex(verts, v3, (float)i/segs, (float)(j+1)/stepsZ);

                // Tam giác 2
                pushVertex(verts, v2, (float)(i+1)/segs, (float)j/stepsZ);
                pushVertex(verts, v4, (float)(i+1)/segs, (float)(j+1)/stepsZ);
                pushVertex(verts, v3, (float)i/segs, (float)(j+1)/stepsZ);
            }
        }

        glassVertexCount = verts.size() / 8;

        glGenVertexArrays(1, &VAO_Glass);
        glGenBuffers(1, &VBO_Glass);
        glBindVertexArray(VAO_Glass);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_Glass);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

        // aPos (3 floats)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // aNormal (3 floats)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // aTexCoords (2 floats)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    // Sinh sàn đi bộ phẳng
    void generateFloor(float width, float l, int stepsZ) {
        std::vector<float> verts;
        float stepZ = l / stepsZ;

        for (int j = 0; j < stepsZ; ++j) {
            float z1 = -l / 2.0f + j * stepZ;
            float z2 = z1 + stepZ;

            glm::vec3 v1(-width, 0.0f, z1);
            glm::vec3 v2(width,  0.0f, z1);
            glm::vec3 v3(-width, 0.0f, z2);
            glm::vec3 v4(width,  0.0f, z2);

            pushFloorVertex(verts, v1);
            pushFloorVertex(verts, v2);
            pushFloorVertex(verts, v3);

            pushFloorVertex(verts, v2);
            pushFloorVertex(verts, v4);
            pushFloorVertex(verts, v3);
        }

        floorVertexCount = verts.size() / 8;

        glGenVertexArrays(1, &VAO_Floor);
        glGenBuffers(1, &VBO_Floor);
        glBindVertexArray(VAO_Floor);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_Floor);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    void pushVertex(std::vector<float>& buf, glm::vec3 pos, float u, float v) {
        buf.push_back(pos.x); buf.push_back(pos.y); buf.push_back(pos.z);
        glm::vec3 norm = glm::normalize(glm::vec3(pos.x, pos.y, 0.0f));
        buf.push_back(norm.x); buf.push_back(norm.y); buf.push_back(norm.z);
        buf.push_back(u); buf.push_back(v);
    }

    void pushFloorVertex(std::vector<float>& buf, glm::vec3 pos) {
        buf.push_back(pos.x); buf.push_back(pos.y); buf.push_back(pos.z);
        buf.push_back(0.0f); buf.push_back(1.0f); buf.push_back(0.0f); // Normal hướng lên trời
        buf.push_back(pos.x * 0.05f); buf.push_back(pos.z * 0.05f);
    }
};
