#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <stb_image.h>
#include "Shader.h"

/**
 * ==============================================================================================
 * 📌 FILE: include/Tunnel.h
 * 👤 PHỤ TRÁCH: BẠN A (Backend / Kỹ sư Toán học & Hình học Tham số)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.5: ĐƯỜNG CONG VÀ MẶT CONG (Curves and Surfaces)
 *      -> Mặt cong tham số (Parametric Surface): Nửa hình trụ tròn nằm dọc trục Z
 *      -> Dầm thép chịu lực 3D khối đặc (Extruded 3D Steel Arches)
 *      -> Vector pháp tuyến giải tích xuyên tâm (Analytical Radial Normals)
 *    - CHƯƠNG CG.6: KỸ THUẬT ĐỒ HỌA NÂNG CAO (Advanced Techniques)
 *      -> Dán vân đá lát sàn (Floor Texture Mapping)
 *      -> Kính Acrylic trong suốt phản quang Fresnel (Alpha Blending: glBlendFunc + glDepthMask)
 * ==============================================================================================
 */

class Tunnel {
public:
    // ------------------------------------------------------------------------------------------
    // [CỤM 1: CÁC ĐỐI TƯỢNG QUẢN LÝ BỘ ĐỆM ĐỒ HỌA OPENGL (VAO & VBO)]
    // ------------------------------------------------------------------------------------------
    unsigned int VAO_Glass = 0, VBO_Glass = 0; // Vòm kính Acrylic trong suốt
    unsigned int VAO_Floor = 0, VBO_Floor = 0; // Sàn đi bộ lát gạch đá
    unsigned int VAO_Curbs = 0, VBO_Curbs = 0; // Bờ gờ bê tông & bệ đỡ hai bên vách kính
    unsigned int VAO_Beams = 0, VBO_Beams = 0; // Khung dầm thép 3D chịu lực uốn cong
    unsigned int VAO_Leds  = 0, VBO_Leds  = 0; // Dải đèn LED cyan phát quang dẫn lối
    unsigned int VAO_Kiosk = 0, VBO_Kiosk = 0; // Các trụ thông tin sinh vật (Kiosk)

    unsigned int floorTexID = 0;
    bool hasFloorTex = false;

    int glassVertexCount = 0;
    int floorVertexCount = 0;
    int curbVertexCount  = 0;
    int beamVertexCount  = 0;
    int ledVertexCount   = 0;
    int kioskVertexCount = 0;

    float Radius;
    float Length;

    Tunnel(float radius = 160.0f, float length = 1600.0f, int segments = 40, int stepsZ = 36)
        : Radius(radius), Length(length) {
        loadFloorTexture("assets/textures/floor_diffuse.jpg");
        generateGlassArch(radius, length, segments, stepsZ);
        generateFloor(radius * 0.78f, length, stepsZ);
        generateSideCurbs(radius * 0.78f, radius, length);
        generateSteelBeams3D(radius, length, segments, 120.0f);
        generateLedStrips(radius * 0.78f, length, stepsZ);
        generateKiosks(radius * 0.70f);
    }

    ~Tunnel() {
        glDeleteVertexArrays(1, &VAO_Glass); glDeleteBuffers(1, &VBO_Glass);
        glDeleteVertexArrays(1, &VAO_Floor); glDeleteBuffers(1, &VBO_Floor);
        glDeleteVertexArrays(1, &VAO_Curbs); glDeleteBuffers(1, &VBO_Curbs);
        glDeleteVertexArrays(1, &VAO_Beams); glDeleteBuffers(1, &VBO_Beams);
        glDeleteVertexArrays(1, &VAO_Leds);  glDeleteBuffers(1, &VBO_Leds);
        glDeleteVertexArrays(1, &VAO_Kiosk); glDeleteBuffers(1, &VBO_Kiosk);
        if (hasFloorTex && floorTexID != 0) glDeleteTextures(1, &floorTexID);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: HÀM VẼ TOÀN BỘ CÔNG TRÌNH HẦM THỦY CUNG (TUÂN THỦ THỨ TỰ OPAQUE -> TRANSPARENT)]
    // ------------------------------------------------------------------------------------------
    void Draw(Shader& shader) {
        shader.setBool("isFish", false);
        shader.setFloat("causticStrength", 0.0f); // Không chiếu gợn sóng nước lên sàn khô trong hầm

        // --- 1. VẼ SÀN HẦM ĐI BỘ (CÓ TEXTURE ĐÁ LÁT) ---
        if (hasFloorTex) {
            shader.setBool("useTexture", true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, floorTexID);
            shader.setInt("texture_diffuse1", 0);
            shader.setVec3("objectColor", glm::vec3(0.55f, 0.62f, 0.70f));
        } else {
            shader.setBool("useTexture", false);
            shader.setVec3("objectColor", glm::vec3(0.10f, 0.14f, 0.20f));
        }
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Floor);
        glDrawArrays(GL_TRIANGLES, 0, floorVertexCount);
        glBindTexture(GL_TEXTURE_2D, 0);

        shader.setBool("useTexture", false);

        // --- 2. VẼ BỜ GỜ BỆ ĐỠ HAI BÊN CHÂN KÍNH ---
        shader.setVec3("objectColor", glm::vec3(0.14f, 0.19f, 0.26f));
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Curbs);
        glDrawArrays(GL_TRIANGLES, 0, curbVertexCount);

        // --- 3. VẼ CÁC TRỤ THÔNG TIN SINH VẬT (KIOSK) ---
        shader.setVec3("objectColor", glm::vec3(0.18f, 0.26f, 0.36f));
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Kiosk);
        glDrawArrays(GL_TRIANGLES, 0, kioskVertexCount);

        // --- 4. VẼ 2 DẢI ĐÈN LED NEON DẪN LỐI ---
        shader.setFloat("causticStrength", 0.0f);
        shader.setVec3("objectColor", glm::vec3(0.12f, 0.96f, 0.90f));
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Leds);
        glDrawArrays(GL_TRIANGLES, 0, ledVertexCount);

        // --- 5. VẼ CÁC KHUNG DẦM THÉP 3D CHỊU LỰC UỐN CONG ---
        shader.setVec3("objectColor", glm::vec3(0.18f, 0.24f, 0.33f));
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Beams);
        glDrawArrays(GL_TRIANGLES, 0, beamVertexCount);

        // --- 6. VẼ VÒM KÍNH ACRYLIC TRONG SUỐT (ALPHA BLENDING + FRESNEL) ---
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        shader.setFloat("causticStrength", 0.35f); // Kính bắt nhẹ ánh khúc xạ nước biển bên ngoài
        shader.setVec3("objectColor", glm::vec3(0.32f, 0.78f, 0.96f));
        shader.setFloat("alphaVal", 0.16f);
        glBindVertexArray(VAO_Glass);
        glDrawArrays(GL_TRIANGLES, 0, glassVertexCount);

        glDepthMask(GL_TRUE);
        glBindVertexArray(0);
    }

private:
    void loadFloorTexture(const std::string& path) {
        int w = 0, h = 0, ch = 0;
        stbi_set_flip_vertically_on_load(1);
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
        if (data) {
            GLenum fmt = (ch == 4) ? GL_RGBA : (ch == 3 ? GL_RGB : GL_RED);
            glGenTextures(1, &floorTexID);
            glBindTexture(GL_TEXTURE_2D, floorTexID);
            glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            stbi_image_free(data);
            hasFloorTex = true;
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: TOÁN HỌC THAM SỐ SINH LƯỚI VÒM KÍNH NỬA HÌNH TRỤ (CHƯƠNG CG.5)]
    // ------------------------------------------------------------------------------------------
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

                pushVertex(verts, v1, (float)i/segs,     (float)j/stepsZ);
                pushVertex(verts, v2, (float)(i+1)/segs, (float)j/stepsZ);
                pushVertex(verts, v3, (float)i/segs,     (float)(j+1)/stepsZ);

                pushVertex(verts, v2, (float)(i+1)/segs, (float)j/stepsZ);
                pushVertex(verts, v4, (float)(i+1)/segs, (float)(j+1)/stepsZ);
                pushVertex(verts, v3, (float)i/segs,     (float)(j+1)/stepsZ);
            }
        }

        // Bổ sung 2 Mái Vòm Kính Bán Cầu Toàn Cảnh ở 2 đầu hầm (Z = -L/2 và Z = +L/2)
        // Khắc phục triệt để hạn chế "hầm bị cắt cụt ở hai đầu"
        const int domeRings = 12;
        const int domeSegs = segs;
        for (int endSign : {-1, 1}) {
            float zCenter = endSign * (l / 2.0f);
            for (int rIdx = 0; rIdx < domeRings; ++rIdx) {
                float lat1 = (3.14159265f * 0.5f) * (float)rIdx / domeRings;
                float lat2 = (3.14159265f * 0.5f) * (float)(rIdx + 1) / domeRings;
                for (int sIdx = 0; sIdx < domeSegs; ++sIdx) {
                    float lon1 = 3.14159265f * (float)sIdx / domeSegs;
                    float lon2 = 3.14159265f * (float)(sIdx + 1) / domeSegs;

                    auto domePt = [&](float lat, float lon) {
                        return glm::vec3(
                            r * cos(lat) * cos(lon),
                            r * sin(lat),
                            zCenter + endSign * r * cos(lat) * sin(lon)
                        );
                    };
                    auto pushDomeV = [&](glm::vec3 p, float u, float v) {
                        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
                        glm::vec3 n = glm::normalize(p - glm::vec3(0.0f, 0.0f, zCenter));
                        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z);
                        verts.push_back(u); verts.push_back(v);
                    };

                    glm::vec3 p1 = domePt(lat1, lon1), p2 = domePt(lat1, lon2);
                    glm::vec3 p3 = domePt(lat2, lon1), p4 = domePt(lat2, lon2);
                    pushDomeV(p1, (float)sIdx/domeSegs, (float)rIdx/domeRings);
                    pushDomeV(p2, (float)(sIdx+1)/domeSegs, (float)rIdx/domeRings);
                    pushDomeV(p3, (float)sIdx/domeSegs, (float)(rIdx+1)/domeRings);

                    pushDomeV(p2, (float)(sIdx+1)/domeSegs, (float)rIdx/domeRings);
                    pushDomeV(p4, (float)(sIdx+1)/domeSegs, (float)(rIdx+1)/domeRings);
                    pushDomeV(p3, (float)sIdx/domeSegs, (float)(rIdx+1)/domeRings);
                }
            }
        }

        glassVertexCount = static_cast<int>(verts.size() / 8);
        setupBuffer(VAO_Glass, VBO_Glass, verts);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 4: SINH SÀN ĐI BỘ LÁT GẠCH ĐÁ]
    // ------------------------------------------------------------------------------------------
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

            pushFloorVertex(verts, v1); pushFloorVertex(verts, v2); pushFloorVertex(verts, v3);
            pushFloorVertex(verts, v2); pushFloorVertex(verts, v4); pushFloorVertex(verts, v3);
        }

        // Sàn bán nguyệt nối liền mạch cho 2 Đài Quan Sát ở đầu và cuối hầm
        const int fanSegs = 28;
        for (int endSign : {-1, 1}) {
            float zCenter = endSign * (l / 2.0f);
            glm::vec3 centerPt(0.0f, 0.0f, zCenter);
            for (int i = 0; i < fanSegs; ++i) {
                float a1 = 3.14159265f * (float)i / fanSegs;
                float a2 = 3.14159265f * (float)(i + 1) / fanSegs;
                glm::vec3 p1(width * cos(a1), 0.0f, zCenter + endSign * width * sin(a1));
                glm::vec3 p2(width * cos(a2), 0.0f, zCenter + endSign * width * sin(a2));
                pushFloorVertex(verts, centerPt);
                pushFloorVertex(verts, p1);
                pushFloorVertex(verts, p2);
            }
        }

        floorVertexCount = static_cast<int>(verts.size() / 8);
        setupBuffer(VAO_Floor, VBO_Floor, verts);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: SINH BỜ GỜ BÊ TÔNG HAI BÊN CHÂN VÒM KÍNH]
    // ------------------------------------------------------------------------------------------
    void generateSideCurbs(float innerX, float outerX, float l) {
        std::vector<float> verts;
        float z1 = -l / 2.0f, z2 = l / 2.0f;
        float h = 8.0f;

        // Helper đẩy 1 hộp chữ nhật
        auto addBox = [&](float xMin, float xMax, float yMin, float yMax, float zMin, float zMax) {
            auto pushTri = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 n) {
                for (glm::vec3 p : {a, b, c}) {
                    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
                    verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z);
                    verts.push_back(p.x * 0.02f); verts.push_back(p.z * 0.02f);
                }
            };
            glm::vec3 p000(xMin, yMin, zMin), p100(xMax, yMin, zMin);
            glm::vec3 p010(xMin, yMax, zMin), p110(xMax, yMax, zMin);
            glm::vec3 p001(xMin, yMin, zMax), p101(xMax, yMin, zMax);
            glm::vec3 p011(xMin, yMax, zMax), p111(xMax, yMax, zMax);
            // Top (+Y)
            pushTri(p010, p110, p011, {0, 1, 0}); pushTri(p110, p111, p011, {0, 1, 0});
            // Inner (-X or +X)
            pushTri(p000, p010, p001, {-1, 0, 0}); pushTri(p010, p011, p001, {-1, 0, 0});
            pushTri(p100, p101, p110, {1, 0, 0});  pushTri(p110, p101, p111, {1, 0, 0});
            // Front/Back
            pushTri(p000, p100, p010, {0, 0, -1}); pushTri(p100, p110, p010, {0, 0, -1});
            pushTri(p001, p011, p101, {0, 0, 1});  pushTri(p101, p011, p111, {0, 0, 1});
        };

        addBox(-outerX, -innerX, 0.0f, h, z1, z2);
        addBox( innerX,  outerX, 0.0f, h, z1, z2);

        curbVertexCount = static_cast<int>(verts.size() / 8);
        setupBuffer(VAO_Curbs, VBO_Curbs, verts);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 6: SINH CÁC DẦM THÉP 3D KHỐI ĐẶC UỐN CONG NÂNG ĐỠ VÒM KÍNH]
    // ------------------------------------------------------------------------------------------
    void generateSteelBeams3D(float r, float l, int segs, float spacing) {
        std::vector<float> verts;
        int beamCount = static_cast<int>(l / spacing);
        float startZ = -l / 2.0f;
        float halfThickZ = 3.2f;
        float rIn = r - 3.5f;
        float rOut = r + 2.5f;

        auto pushFlatTri = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c) {
            glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
            if (glm::length(n) < 1e-5f) n = glm::vec3(0, 1, 0);
            for (glm::vec3 p : {a, b, c}) {
                verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
                verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z);
                verts.push_back(0.0f); verts.push_back(0.0f);
            }
        };

        for (int b = 0; b <= beamCount; ++b) {
            float zc = startZ + b * spacing;
            float z1 = zc - halfThickZ;
            float z2 = zc + halfThickZ;

            for (int i = 0; i < segs; ++i) {
                float t1 = 3.14159265f * (float)i / segs;
                float t2 = 3.14159265f * (float)(i + 1) / segs;

                glm::vec3 i1(rIn  * cos(t1), rIn  * sin(t1), z1);
                glm::vec3 i2(rIn  * cos(t2), rIn  * sin(t2), z1);
                glm::vec3 i3(rIn  * cos(t1), rIn  * sin(t1), z2);
                glm::vec3 i4(rIn  * cos(t2), rIn  * sin(t2), z2);

                glm::vec3 o1(rOut * cos(t1), rOut * sin(t1), z1);
                glm::vec3 o2(rOut * cos(t2), rOut * sin(t2), z1);
                glm::vec3 o3(rOut * cos(t1), rOut * sin(t1), z2);
                glm::vec3 o4(rOut * cos(t2), rOut * sin(t2), z2);

                // Mặt trong hướng xuống lối đi
                pushFlatTri(i1, i2, i3); pushFlatTri(i2, i4, i3);
                // Mặt ngoài
                pushFlatTri(o1, o3, o2); pushFlatTri(o2, o3, o4);
                // Mặt bên trước (-Z) và sau (+Z)
                pushFlatTri(i1, o1, i2); pushFlatTri(i2, o1, o2);
                pushFlatTri(i3, i4, o3); pushFlatTri(i4, o4, o3);
            }
        }

        beamVertexCount = static_cast<int>(verts.size() / 8);
        setupBuffer(VAO_Beams, VBO_Beams, verts);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 7: SINH 2 DẢI ĐÈN LED NEON DẪN LỐI & 4 TRỤ THÔNG TIN KIOSK]
    // ------------------------------------------------------------------------------------------
    void generateLedStrips(float edgeX, float l, int stepsZ) {
        std::vector<float> verts;
        float stepZ = l / stepsZ;
        float ledWidth = 4.2f;

        for (int j = 0; j < stepsZ; ++j) {
            float z1 = -l / 2.0f + j * stepZ;
            float z2 = z1 + stepZ;

            glm::vec3 l1(-edgeX, 0.3f, z1), l2(-edgeX + ledWidth, 0.3f, z1);
            glm::vec3 l3(-edgeX, 0.3f, z2), l4(-edgeX + ledWidth, 0.3f, z2);
            pushFloorVertex(verts, l1); pushFloorVertex(verts, l2); pushFloorVertex(verts, l3);
            pushFloorVertex(verts, l2); pushFloorVertex(verts, l4); pushFloorVertex(verts, l3);

            glm::vec3 r1(edgeX - ledWidth, 0.3f, z1), r2(edgeX, 0.3f, z1);
            glm::vec3 r3(edgeX - ledWidth, 0.3f, z2), r4(edgeX, 0.3f, z2);
            pushFloorVertex(verts, r1); pushFloorVertex(verts, r2); pushFloorVertex(verts, r3);
            pushFloorVertex(verts, r2); pushFloorVertex(verts, r4); pushFloorVertex(verts, r3);
        }

        // Vòng đèn LED bán nguyệt ôm trọn chu vi 2 Đài Quan Sát cuối hầm
        const int arcSegs = 28;
        for (int endSign : {-1, 1}) {
            float zCenter = endSign * (l / 2.0f);
            float rIn = edgeX - ledWidth;
            float rOut = edgeX;
            for (int i = 0; i < arcSegs; ++i) {
                float a1 = 3.14159265f * (float)i / arcSegs;
                float a2 = 3.14159265f * (float)(i + 1) / arcSegs;
                glm::vec3 i1(rIn  * cos(a1), 0.3f, zCenter + endSign * rIn  * sin(a1));
                glm::vec3 i2(rIn  * cos(a2), 0.3f, zCenter + endSign * rIn  * sin(a2));
                glm::vec3 o1(rOut * cos(a1), 0.3f, zCenter + endSign * rOut * sin(a1));
                glm::vec3 o2(rOut * cos(a2), 0.3f, zCenter + endSign * rOut * sin(a2));
                pushFloorVertex(verts, i1); pushFloorVertex(verts, o1); pushFloorVertex(verts, i2);
                pushFloorVertex(verts, o1); pushFloorVertex(verts, o2); pushFloorVertex(verts, i2);
            }
        }

        ledVertexCount = static_cast<int>(verts.size() / 8);
        setupBuffer(VAO_Leds, VBO_Leds, verts);
    }

    void generateKiosks(float sideX) {
        std::vector<float> verts;
        auto addBox = [&](float cx, float cy, float cz, float wx, float wy, float wz) {
            float x0 = cx - wx * 0.5f, x1 = cx + wx * 0.5f;
            float y0 = cy,             y1 = cy + wy;
            float z0 = cz - wz * 0.5f, z1 = cz + wz * 0.5f;
            auto pushTri = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 n) {
                for (glm::vec3 p : {a, b, c}) {
                    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
                    verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z);
                    verts.push_back(0.0f); verts.push_back(0.0f);
                }
            };
            glm::vec3 p000(x0,y0,z0), p100(x1,y0,z0), p010(x0,y1,z0), p110(x1,y1,z0);
            glm::vec3 p001(x0,y0,z1), p101(x1,y0,z1), p011(x0,y1,z1), p111(x1,y1,z1);
            pushTri(p010, p110, p011, {0,1,0}); pushTri(p110, p111, p011, {0,1,0});
            pushTri(p000, p010, p001, {-1,0,0}); pushTri(p010, p011, p001, {-1,0,0});
            pushTri(p100, p101, p110, {1,0,0});  pushTri(p110, p101, p111, {1,0,0});
            pushTri(p000, p100, p010, {0,0,-1}); pushTri(p100, p110, p010, {0,0,-1});
            pushTri(p001, p011, p101, {0,0,1});  pushTri(p101, p011, p111, {0,0,1});
        };

        float zPositions[] = {-420.0f, -120.0f, 180.0f, 480.0f};
        for (int i = 0; i < 4; ++i) {
            float sx = (i % 2 == 0) ? -sideX : sideX;
            addBox(sx, 0.0f, zPositions[i], 14.0f, 26.0f, 22.0f);
        }

        kioskVertexCount = static_cast<int>(verts.size() / 8);
        setupBuffer(VAO_Kiosk, VBO_Kiosk, verts);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 8: HÀM NẠP DỮ LIỆU ĐỈNH VÀO BỘ ĐỆM VRAM GPU (VAO / VBO)]
    // ------------------------------------------------------------------------------------------
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
        buf.push_back(pos.x * 0.025f); buf.push_back(pos.z * 0.025f);
    }
};
