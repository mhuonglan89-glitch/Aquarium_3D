#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include "Shader.h"

/**
 * @file Tunnel.h
 * @brief Sinh lưới vòm kính hình trụ, dầm thép chịu lực, dải đèn LED và sàn hầm bằng toán học tham số
 * @author Bạn A (Backend / Math & Geometry) - Môn Đồ Họa Máy Tính (Chương CG.5: Curves and Surfaces)
 */
class Tunnel {
public:
    unsigned int VAO_Glass, VBO_Glass;
    unsigned int VAO_Floor, VBO_Floor;
    unsigned int VAO_Beams, VBO_Beams;
    unsigned int VAO_Leds,  VBO_Leds;

    int glassVertexCount = 0;
    int floorVertexCount = 0;
    int beamVertexCount  = 0;
    int ledVertexCount   = 0;

    float Radius;
    float Length;

    Tunnel(float radius = 160.0f, float length = 1500.0f, int segments = 32, int stepsZ = 30)
        : Radius(radius), Length(length) {
        
        generateGlassArch(radius, length, segments, stepsZ);
        generateFloor(radius * 0.75f, length, stepsZ);
        generateSteelBeams(radius, length, segments, 100.0f);
        generateLedStrips(radius * 0.73f, length, stepsZ);
    }

    ~Tunnel() {
        glDeleteVertexArrays(1, &VAO_Glass);
        glDeleteBuffers(1, &VBO_Glass);
        glDeleteVertexArrays(1, &VAO_Floor);
        glDeleteBuffers(1, &VBO_Floor);
        glDeleteVertexArrays(1, &VAO_Beams);
        glDeleteBuffers(1, &VBO_Beams);
        glDeleteVertexArrays(1, &VAO_Leds);
        glDeleteBuffers(1, &VBO_Leds);
    }

    // Vẽ toàn bộ hầm thủy cung
    void Draw(Shader& shader) {
        shader.setBool("useTexture", false);
        // 1. Vẽ sàn gạch hầm tối màu (Titanium Steel #1E293B)
        shader.setVec3("objectColor", glm::vec3(0.08f, 0.12f, 0.18f));
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Floor);
        glDrawArrays(GL_TRIANGLES, 0, floorVertexCount);

        // 2. Vẽ dải đèn LED xanh dẫn lối (Bioluminescent Cyan #0CE8DA)
        shader.setVec3("objectColor", glm::vec3(0.047f, 0.910f, 0.855f));
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Leds);
        glDrawArrays(GL_TRIANGLES, 0, ledVertexCount);

        // 3. Vẽ các dầm thép chịu lực uốn cong đỡ vòm kính (Titanium Steel)
        shader.setVec3("objectColor", glm::vec3(0.18f, 0.22f, 0.30f));
        shader.setFloat("alphaVal", 1.0f);
        glLineWidth(3.0f);
        glBindVertexArray(VAO_Beams);
        glDrawArrays(GL_LINES, 0, beamVertexCount);

        // 4. Vẽ vòm kính thủy cung (Bật chế độ trong suốt Alpha Blending - Chương CG.6)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Tạm dừng ghi depth khi vẽ kính trong suốt
        
        shader.setVec3("objectColor", glm::vec3(0.15f, 0.45f, 0.65f));
        shader.setFloat("alphaVal", 0.30f); // Độ trong suốt 30% kính Acrylic
        glBindVertexArray(VAO_Glass);
        glDrawArrays(GL_TRIANGLES, 0, glassVertexCount);

        glDepthMask(GL_TRUE); // Khôi phục ghi depth
        glBindVertexArray(0);
    }

private:
    // Sinh lưới vòm kính nửa hình trụ: x = R*cos(theta), y = R*sin(theta), z in [-L/2, L/2]
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
        setupBuffer(VAO_Glass, VBO_Glass, verts);
    }

    // Sinh sàn đi bộ phẳng với gạch tối màu
    void generateFloor(float width, float l, int stepsZ) {
        std::vector<float> verts;
        float stepZ = l / stepsZ;

        for (int j = 0; j < stepsZ; ++j) {
            float z1 = -l / 2.0f + j * stepZ;
            float z2 = z1 + stepZ;

            glm::vec3 v1(-width, 0.0f, z1);
            glm::vec3 v2( width, 0.0f, z1);
            glm::vec3 v3(-width, 0.0f, z2);
            glm::vec3 v4( width, 0.0f, z2);

            pushFloorVertex(verts, v1);
            pushFloorVertex(verts, v2);
            pushFloorVertex(verts, v3);

            pushFloorVertex(verts, v2);
            pushFloorVertex(verts, v4);
            pushFloorVertex(verts, v3);
        }

        floorVertexCount = verts.size() / 8;
        setupBuffer(VAO_Floor, VBO_Floor, verts);
    }

    // Sinh các dầm thép chịu lực uốn cong nâng đỡ vòm kính
    void generateSteelBeams(float r, float l, int segs, float spacing) {
        std::vector<float> verts;
        int beamCount = static_cast<int>(l / spacing);
        float startZ = -l / 2.0f;

        for (int b = 0; b <= beamCount; ++b) {
            float z = startZ + b * spacing;
            for (int i = 0; i < segs; ++i) {
                float t1 = 3.14159265f * (float)i / segs;
                float t2 = 3.14159265f * (float)(i + 1) / segs;

                glm::vec3 p1(r * cos(t1), r * sin(t1), z);
                glm::vec3 p2(r * cos(t2), r * sin(t2), z);

                pushVertex(verts, p1, 0.0f, 0.0f);
                pushVertex(verts, p2, 1.0f, 0.0f);
            }
        }

        beamVertexCount = verts.size() / 8;
        setupBuffer(VAO_Beams, VBO_Beams, verts);
    }

    // Sinh 2 dải đèn LED cyan neon dẫn lối dọc theo mép sàn hầm
    void generateLedStrips(float edgeX, float l, int stepsZ) {
        std::vector<float> verts;
        float stepZ = l / stepsZ;
        float ledWidth = 4.0f; // Bề rộng dải LED 4 đơn vị

        for (int j = 0; j < stepsZ; ++j) {
            float z1 = -l / 2.0f + j * stepZ;
            float z2 = z1 + stepZ;

            // Dải LED bên trái
            glm::vec3 l1(-edgeX, 0.2f, z1);
            glm::vec3 l2(-edgeX + ledWidth, 0.2f, z1);
            glm::vec3 l3(-edgeX, 0.2f, z2);
            glm::vec3 l4(-edgeX + ledWidth, 0.2f, z2);

            pushFloorVertex(verts, l1);
            pushFloorVertex(verts, l2);
            pushFloorVertex(verts, l3);
            pushFloorVertex(verts, l2);
            pushFloorVertex(verts, l4);
            pushFloorVertex(verts, l3);

            // Dải LED bên phải
            glm::vec3 r1(edgeX - ledWidth, 0.2f, z1);
            glm::vec3 r2(edgeX, 0.2f, z1);
            glm::vec3 r3(edgeX - ledWidth, 0.2f, z2);
            glm::vec3 r4(edgeX, 0.2f, z2);

            pushFloorVertex(verts, r1);
            pushFloorVertex(verts, r2);
            pushFloorVertex(verts, r3);
            pushFloorVertex(verts, r2);
            pushFloorVertex(verts, r4);
            pushFloorVertex(verts, r3);
        }

        ledVertexCount = verts.size() / 8;
        setupBuffer(VAO_Leds, VBO_Leds, verts);
    }

    void setupBuffer(unsigned int& vao, unsigned int& vbo, const std::vector<float>& verts) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    void pushVertex(std::vector<float>& buf, glm::vec3 pos, float u, float v) {
        buf.push_back(pos.x); buf.push_back(pos.y); buf.push_back(pos.z);
        glm::vec3 norm = glm::normalize(glm::vec3(pos.x, pos.y, 0.0f));
        buf.push_back(norm.x); buf.push_back(norm.y); buf.push_back(norm.z);
        buf.push_back(u); buf.push_back(v);
    }

    void pushFloorVertex(std::vector<float>& buf, glm::vec3 pos) {
        buf.push_back(pos.x); buf.push_back(pos.y); buf.push_back(pos.z);
        buf.push_back(0.0f); buf.push_back(1.0f); buf.push_back(0.0f);
        buf.push_back(pos.x * 0.05f); buf.push_back(pos.z * 0.05f);
    }
};
