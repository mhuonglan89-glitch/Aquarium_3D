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
 *      -> Mặt cong tham số phức hợp: Đại Sảnh Trung Tâm 360° + Mê Cung Thủy Cung 4 Nhánh Rẽ
 *      -> Dầm thép chịu lực 3D khối đặc (Extruded 3D Steel Arches) & Cổng vòm giao lộ
 *      -> Mái vòm kính bán cầu toàn cảnh tại các điểm quan sát cuối nhánh
 *    - CHƯƠNG CG.6: KỸ THUẬT ĐỒ HỌA NÂNG CAO (Advanced Techniques)
 *      -> Dán vân đá lát sàn (Floor Texture Mapping)
 *      -> Kính Acrylic trong suốt phản quang Fresnel (Alpha Blending: glBlendFunc + glDepthMask)
 * ==============================================================================================
 */

class Tunnel {
public:
    unsigned int VAO_Glass = 0, VBO_Glass = 0;
    unsigned int VAO_Floor = 0, VBO_Floor = 0;
    unsigned int VAO_Curbs = 0, VBO_Curbs = 0;
    unsigned int VAO_Beams = 0, VBO_Beams = 0;
    unsigned int VAO_Leds  = 0, VBO_Leds  = 0;
    unsigned int VAO_Kiosk = 0, VBO_Kiosk = 0;

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

    Tunnel(float radius = 128.0f, float length = 1600.0f)
        : Radius(radius), Length(length) {
        loadFloorTexture("assets/textures/floor_diffuse.jpg");

        std::vector<float> glassVerts, floorVerts, curbVerts, beamVerts, ledVerts, kioskVerts;

        // 1. ĐẠI SẢNH QUAN SÁT TRUNG TÂM 360° tại (0, 0) - Bán kính 220, Tường kính cao 128 khớp khít với vòm hành lang!
        buildCentralRotunda(0.0f, 0.0f, 220.0f, radius,
                            glassVerts, floorVerts, curbVerts, beamVerts, ledVerts, kioskVerts);

        // 2. NHÁNH NAM (Lối vào chính): (0, -680) -> (0, -195) (nối sâu vào viền sảnh cho khít)
        buildCorridor(glm::vec2(0.0f, -680.0f), glm::vec2(0.0f, -195.0f), radius, true, false,
                      glassVerts, floorVerts, curbVerts, beamVerts, ledVerts, kioskVerts);

        // 3. NHÁNH BẮC -> RẼ PHẢI (Hẻm Cá Mập): (0, 195) -> (0, 680) -> (480, 680)
        buildCorridor(glm::vec2(0.0f, 195.0f), glm::vec2(0.0f, 560.0f), radius, false, false,
                      glassVerts, floorVerts, curbVerts, beamVerts, ledVerts, kioskVerts);
        buildCornerHub(0.0f, 680.0f, radius, -1.0f, 1.0f,
                       glassVerts, floorVerts, curbVerts, beamVerts, ledVerts);
        buildCorridor(glm::vec2(120.0f, 680.0f), glm::vec2(480.0f, 680.0f), radius, false, true,
                      glassVerts, floorVerts, curbVerts, beamVerts, ledVerts, kioskVerts);

        // 4. NHÁNH TÂY -> RẼ TRÁI (Vườn San Hô): (-195, 0) -> (-640, 0) -> (-640, -480)
        buildCorridor(glm::vec2(-195.0f, 0.0f), glm::vec2(-520.0f, 0.0f), radius, false, false,
                      glassVerts, floorVerts, curbVerts, beamVerts, ledVerts, kioskVerts);
        buildCornerHub(-640.0f, 0.0f, radius, 1.0f, -1.0f,
                       glassVerts, floorVerts, curbVerts, beamVerts, ledVerts);
        buildCorridor(glm::vec2(-640.0f, -120.0f), glm::vec2(-640.0f, -480.0f), radius, false, true,
                      glassVerts, floorVerts, curbVerts, beamVerts, ledVerts, kioskVerts);

        // 5. NHÁNH ĐÔNG -> RẼ PHẢI (Rừng Sứa Phát Sáng): (195, 0) -> (640, 0) -> (640, -460)
        buildCorridor(glm::vec2(195.0f, 0.0f), glm::vec2(520.0f, 0.0f), radius, false, false,
                      glassVerts, floorVerts, curbVerts, beamVerts, ledVerts, kioskVerts);
        buildCornerHub(640.0f, 0.0f, radius, -1.0f, -1.0f,
                       glassVerts, floorVerts, curbVerts, beamVerts, ledVerts);
        buildCorridor(glm::vec2(640.0f, -120.0f), glm::vec2(640.0f, -460.0f), radius, false, true,
                      glassVerts, floorVerts, curbVerts, beamVerts, ledVerts, kioskVerts);

        glassVertexCount = static_cast<int>(glassVerts.size() / 8);
        floorVertexCount = static_cast<int>(floorVerts.size() / 8);
        curbVertexCount  = static_cast<int>(curbVerts.size() / 8);
        beamVertexCount  = static_cast<int>(beamVerts.size() / 8);
        ledVertexCount   = static_cast<int>(ledVerts.size() / 8);
        kioskVertexCount = static_cast<int>(kioskVerts.size() / 8);

        setupBuffer(VAO_Glass, VBO_Glass, glassVerts);
        setupBuffer(VAO_Floor, VBO_Floor, floorVerts);
        setupBuffer(VAO_Curbs, VBO_Curbs, curbVerts);
        setupBuffer(VAO_Beams, VBO_Beams, beamVerts);
        setupBuffer(VAO_Leds,  VBO_Leds,  ledVerts);
        setupBuffer(VAO_Kiosk, VBO_Kiosk, kioskVerts);
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
    // [CỤM 2: HÀM VẼ TOÀN BỘ KHU PHỨC HỢP THỦY CUNG (OPAQUE -> TRANSPARENT)]
    // ------------------------------------------------------------------------------------------
    void Draw(Shader& shader) {
        shader.setBool("isFish", false);
        shader.setFloat("causticStrength", 0.0f);

        // 1. Sàn đá lát toàn bộ mê cung & đại sảnh
        if (hasFloorTex) {
            shader.setBool("useTexture", true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, floorTexID);
            shader.setInt("texture_diffuse1", 0);
            shader.setVec3("objectColor", glm::vec3(0.60f, 0.68f, 0.76f));
        } else {
            shader.setBool("useTexture", false);
            shader.setVec3("objectColor", glm::vec3(0.10f, 0.14f, 0.20f));
        }
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Floor);
        glDrawArrays(GL_TRIANGLES, 0, floorVertexCount);
        glBindTexture(GL_TEXTURE_2D, 0);

        shader.setBool("useTexture", false);

        // 2. Bờ gờ bê tông
        shader.setVec3("objectColor", glm::vec3(0.15f, 0.20f, 0.28f));
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Curbs);
        glDrawArrays(GL_TRIANGLES, 0, curbVertexCount);

        // 3. Các trụ thông tin Kiosk & Bục trung tâm
        shader.setVec3("objectColor", glm::vec3(0.18f, 0.28f, 0.40f));
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Kiosk);
        glDrawArrays(GL_TRIANGLES, 0, kioskVertexCount);

        // 4. Dải đèn LED Neon Cyan dẫn lối rẽ trái/phải
        shader.setVec3("objectColor", glm::vec3(0.10f, 0.96f, 0.92f));
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Leds);
        glDrawArrays(GL_TRIANGLES, 0, ledVertexCount);

        // 5. Khung dầm thép 3D chịu lực
        shader.setVec3("objectColor", glm::vec3(0.19f, 0.25f, 0.35f));
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Beams);
        glDrawArrays(GL_TRIANGLES, 0, beamVertexCount);

        // 6. Vòm kính Acrylic trong suốt (Alpha Blending + Fresnel)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        shader.setFloat("causticStrength", 0.25f);
        shader.setVec3("objectColor", glm::vec3(0.35f, 0.80f, 0.96f));
        shader.setFloat("alphaVal", 0.11f); // Kính trong suốt cao (11%) giúp quan sát cá cực rõ!
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

    static void pushP(std::vector<float>& buf, glm::vec3 pos, glm::vec3 norm, float u, float v) {
        buf.push_back(pos.x); buf.push_back(pos.y); buf.push_back(pos.z);
        buf.push_back(norm.x); buf.push_back(norm.y); buf.push_back(norm.z);
        buf.push_back(u); buf.push_back(v);
    }

    static void pushFlatTri(std::vector<float>& buf, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
        glm::vec3 cr = glm::cross(b - a, c - a);
        glm::vec3 n = (glm::length(cr) > 1e-5f) ? glm::normalize(cr) : glm::vec3(0, 1, 0);
        pushP(buf, a, n, a.x * 0.02f, a.z * 0.02f);
        pushP(buf, b, n, b.x * 0.02f, b.z * 0.02f);
        pushP(buf, c, n, c.x * 0.02f, c.z * 0.02f);
    }

    static void addBox(std::vector<float>& buf, glm::vec3 c, glm::vec3 halfExt) {
        float x0 = c.x - halfExt.x, x1 = c.x + halfExt.x;
        float y0 = c.y - halfExt.y, y1 = c.y + halfExt.y;
        float z0 = c.z - halfExt.z, z1 = c.z + halfExt.z;
        glm::vec3 p000(x0,y0,z0), p100(x1,y0,z0), p010(x0,y1,z0), p110(x1,y1,z0);
        glm::vec3 p001(x0,y0,z1), p101(x1,y0,z1), p011(x0,y1,z1), p111(x1,y1,z1);
        pushFlatTri(buf, p010, p110, p011); pushFlatTri(buf, p110, p111, p011);
        pushFlatTri(buf, p000, p010, p001); pushFlatTri(buf, p010, p011, p001);
        pushFlatTri(buf, p100, p101, p110); pushFlatTri(buf, p110, p101, p111);
        pushFlatTri(buf, p000, p100, p010); pushFlatTri(buf, p100, p110, p010);
        pushFlatTri(buf, p001, p011, p101); pushFlatTri(buf, p101, p011, p111);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: XÂY DỰNG ĐẠI SẢNH QUAN SÁT TRUNG TÂM 360° TẠI (0, 0)]
    // ------------------------------------------------------------------------------------------
    void buildCentralRotunda(float cx, float cz, float R, float tunnelR,
                             std::vector<float>& glass, std::vector<float>& floor,
                             std::vector<float>& curbs, std::vector<float>& beams,
                             std::vector<float>& leds,  std::vector<float>& kiosks) {
        const int segs = 48;
        const int domeRings = 10;
        const float wallH = tunnelR; // Khớp chiều cao bằng đúng đỉnh vòm hành lang (128)

        // Sàn tròn trung tâm + Vòng LED phát sáng bao quanh đảo trung tâm
        for (int i = 0; i < segs; ++i) {
            float a1 = 2.0f * 3.14159265f * (float)i / segs;
            float a2 = 2.0f * 3.14159265f * (float)(i + 1) / segs;

            glm::vec3 cPt(cx, 0.0f, cz);
            glm::vec3 p1(cx + R * cosf(a1), 0.0f, cz + R * sinf(a1));
            glm::vec3 p2(cx + R * cosf(a2), 0.0f, cz + R * sinf(a2));
            pushP(floor, cPt, {0,1,0}, 0.5f, 0.5f);
            pushP(floor, p1,  {0,1,0}, p1.x*0.025f, p1.z*0.025f);
            pushP(floor, p2,  {0,1,0}, p2.x*0.025f, p2.z*0.025f);

            // Vòng LED trung tâm bán kính 48..54
            float rIn = 46.0f, rOut = 52.0f;
            glm::vec3 l1(cx + rIn  * cosf(a1), 0.35f, cz + rIn  * sinf(a1));
            glm::vec3 l2(cx + rIn  * cosf(a2), 0.35f, cz + rIn  * sinf(a2));
            glm::vec3 l3(cx + rOut * cosf(a1), 0.35f, cz + rOut * sinf(a1));
            glm::vec3 l4(cx + rOut * cosf(a2), 0.35f, cz + rOut * sinf(a2));
            pushFlatTri(leds, l1, l3, l2);
            pushFlatTri(leds, l2, l3, l4);
        }

        // Bục thông tin trung tâm hình bát giác
        addBox(kiosks, glm::vec3(cx, 12.0f, cz), glm::vec3(24.0f, 12.0f, 24.0f));
        addBox(leds,   glm::vec3(cx, 25.0f, cz), glm::vec3(25.0f,  1.2f, 25.0f));

        // Tường kính trụ tròn (chừa 4 cửa vòm thông sang 4 hướng Bắc, Nam, Đông, Tây)
        for (int i = 0; i < segs; ++i) {
            float a1 = 2.0f * 3.14159265f * (float)i / segs;
            float a2 = 2.0f * 3.14159265f * (float)(i + 1) / segs;
            float aMid = 0.5f * (a1 + a2);

            float xm = R * cosf(aMid), zm = R * sinf(aMid);
            // Bỏ qua các ô nằm ngay trước miệng 4 hành lang (|x| < 102 hoặc |z| < 102)
            bool isDoorway = (std::abs(xm) < tunnelR * 0.92f) || (std::abs(zm) < tunnelR * 0.92f);

            if (!isDoorway) {
                glm::vec3 b1(cx + R * cosf(a1), 0.0f,  cz + R * sinf(a1));
                glm::vec3 b2(cx + R * cosf(a2), 0.0f,  cz + R * sinf(a2));
                glm::vec3 t1(cx + R * cosf(a1), wallH, cz + R * sinf(a1));
                glm::vec3 t2(cx + R * cosf(a2), wallH, cz + R * sinf(a2));
                glm::vec3 n1 = glm::normalize(glm::vec3(cosf(a1), 0.0f, sinf(a1)));
                glm::vec3 n2 = glm::normalize(glm::vec3(cosf(a2), 0.0f, sinf(a2)));
                pushP(glass, b1, n1, (float)i/segs, 0); pushP(glass, b2, n2, (float)(i+1)/segs, 0); pushP(glass, t1, n1, (float)i/segs, 1);
                pushP(glass, b2, n2, (float)(i+1)/segs, 0); pushP(glass, t2, n2, (float)(i+1)/segs, 1); pushP(glass, t1, n1, (float)i/segs, 1);

                // Bờ gờ chân kính vòng cung
                glm::vec3 cb1(cx + (R-18)*cosf(a1), 0, cz + (R-18)*sinf(a1));
                glm::vec3 cb2(cx + (R-18)*cosf(a2), 0, cz + (R-18)*sinf(a2));
                glm::vec3 ct1(cx + (R-18)*cosf(a1), 8, cz + (R-18)*sinf(a1));
                glm::vec3 ct2(cx + (R-18)*cosf(a2), 8, cz + (R-18)*sinf(a2));
                pushFlatTri(curbs, cb1, ct1, cb2); pushFlatTri(curbs, cb2, ct1, ct2);
                pushFlatTri(curbs, ct1, glm::vec3(b1.x, 8, b1.z), ct2);
                pushFlatTri(curbs, ct2, glm::vec3(b1.x, 8, b1.z), glm::vec3(b2.x, 8, b2.z));
            }
        }

        // 4 Cột thép chịu lực đặt tại đúng 4 góc chéo 45°, 135°, 225°, 315° (KHÔNG BAO GIỜ chắn cửa vòm!)
        for (int k = 0; k < 4; ++k) {
            float ang = 0.785398f + k * 1.570796f;
            addBox(beams, glm::vec3(cx + R * cosf(ang), wallH * 0.5f, cz + R * sinf(ang)),
                   glm::vec3(5.0f, wallH * 0.5f, 5.0f));
        }

        // Vòng dầm thép tròn 3D trên cao (Y = wallH) đỡ mái vòm trung tâm
        for (int i = 0; i < segs; ++i) {
            float a1 = 2.0f * 3.14159265f * (float)i / segs;
            float a2 = 2.0f * 3.14159265f * (float)(i + 1) / segs;
            float rIn = R - 3.5f, rOut = R + 3.5f, yB = wallH - 3.0f, yT = wallH + 3.0f;
            glm::vec3 i1(cx + rIn * cosf(a1), yB, cz + rIn * sinf(a1));
            glm::vec3 i2(cx + rIn * cosf(a2), yB, cz + rIn * sinf(a2));
            glm::vec3 i3(cx + rIn * cosf(a1), yT, cz + rIn * sinf(a1));
            glm::vec3 i4(cx + rIn * cosf(a2), yT, cz + rIn * sinf(a2));
            glm::vec3 o1(cx + rOut * cosf(a1), yB, cz + rOut * sinf(a1));
            glm::vec3 o2(cx + rOut * cosf(a2), yB, cz + rOut * sinf(a2));
            glm::vec3 o3(cx + rOut * cosf(a1), yT, cz + rOut * sinf(a1));
            glm::vec3 o4(cx + rOut * cosf(a2), yT, cz + rOut * sinf(a2));
            pushFlatTri(beams, i1, i3, i2); pushFlatTri(beams, i2, i3, i4);
            pushFlatTri(beams, o1, o2, o3); pushFlatTri(beams, o2, o4, o3);
            pushFlatTri(beams, i3, o3, i4); pushFlatTri(beams, i4, o3, o4);
            pushFlatTri(beams, i1, i2, o1); pushFlatTri(beams, i2, o2, o1);
        }

        // Mái vòm bán cầu kính trên đỉnh Đại Sảnh (từ Y = wallH lên Y = wallH + 115)
        for (int rIdx = 0; rIdx < domeRings; ++rIdx) {
            float lat1 = (3.14159265f * 0.5f) * (float)rIdx / domeRings;
            float lat2 = (3.14159265f * 0.5f) * (float)(rIdx + 1) / domeRings;
            for (int sIdx = 0; sIdx < segs; ++sIdx) {
                float lon1 = 2.0f * 3.14159265f * (float)sIdx / segs;
                float lon2 = 2.0f * 3.14159265f * (float)(sIdx + 1) / segs;
                auto dPt = [&](float lat, float lon) {
                    return glm::vec3(cx + R * cosf(lat) * cosf(lon),
                                     wallH + (R * 0.55f) * sinf(lat),
                                     cz + R * cosf(lat) * sinf(lon));
                };
                auto pushDV = [&](glm::vec3 p, float u, float v) {
                    glm::vec3 n = glm::normalize(p - glm::vec3(cx, wallH, cz));
                    pushP(glass, p, n, u, v);
                };
                glm::vec3 p1 = dPt(lat1, lon1), p2 = dPt(lat1, lon2);
                glm::vec3 p3 = dPt(lat2, lon1), p4 = dPt(lat2, lon2);
                pushDV(p1, 0, 0); pushDV(p2, 1, 0); pushDV(p3, 0, 1);
                pushDV(p2, 1, 0); pushDV(p4, 1, 1); pushDV(p3, 0, 1);
            }
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 4: XÂY DỰNG ĐOẠN HÀNH LANG THẲNG THEO HƯỚNG BẤT KỲ (P1 -> P2) + VÒM ĐẦU CUỐI]
    // ------------------------------------------------------------------------------------------
    void buildCorridor(glm::vec2 pStart, glm::vec2 pEnd, float r, bool capStart, bool capEnd,
                       std::vector<float>& glass, std::vector<float>& floor,
                       std::vector<float>& curbs, std::vector<float>& beams,
                       std::vector<float>& leds,  std::vector<float>& kiosks) {
        glm::vec2 diff = pEnd - pStart;
        float len = glm::length(diff);
        if (len < 1.0f) return;
        glm::vec2 fwd2 = diff / len;
        glm::vec2 right2(fwd2.y, -fwd2.x); // Vector vuông góc sang phải

        glm::vec3 fwd(fwd2.x, 0.0f, fwd2.y);
        glm::vec3 right(right2.x, 0.0f, right2.y);
        glm::vec3 origin(pStart.x, 0.0f, pStart.y);

        const int segs = 28;
        const int steps = std::max(4, static_cast<int>(len / 45.0f));
        float stepLen = len / steps;
        float walkW = r * 0.78f;

        // 1. Vòm kính nửa hình trụ
        for (int i = 0; i < segs; ++i) {
            float t1 = 3.14159265f * (float)i / segs;
            float t2 = 3.14159265f * (float)(i + 1) / segs;
            for (int j = 0; j < steps; ++j) {
                float d1 = j * stepLen, d2 = (j + 1) * stepLen;
                auto pt = [&](float theta, float d) {
                    return origin + fwd * d + right * (r * cosf(theta)) + glm::vec3(0.0f, r * sinf(theta), 0.0f);
                };
                auto norm = [&](float theta) {
                    return glm::normalize(right * cosf(theta) + glm::vec3(0.0f, sinf(theta), 0.0f));
                };
                glm::vec3 v1 = pt(t1, d1), v2 = pt(t2, d1), v3 = pt(t1, d2), v4 = pt(t2, d2);
                glm::vec3 n1 = norm(t1), n2 = norm(t2);
                pushP(glass, v1, n1, (float)i/segs, (float)j/steps);
                pushP(glass, v2, n2, (float)(i+1)/segs, (float)j/steps);
                pushP(glass, v3, n1, (float)i/segs, (float)(j+1)/steps);
                pushP(glass, v2, n2, (float)(i+1)/segs, (float)j/steps);
                pushP(glass, v4, n2, (float)(i+1)/segs, (float)(j+1)/steps);
                pushP(glass, v3, n1, (float)i/segs, (float)(j+1)/steps);
            }
        }

        // 2. Sàn đá, Dải đèn LED & Bờ gờ hai bên
        for (int j = 0; j < steps; ++j) {
            float d1 = j * stepLen, d2 = (j + 1) * stepLen;
            auto fPt = [&](float xOff, float y, float d) {
                return origin + fwd * d + right * xOff + glm::vec3(0.0f, y, 0.0f);
            };
            // Sàn đi bộ
            glm::vec3 f1 = fPt(-walkW, 0, d1), f2 = fPt(walkW, 0, d1);
            glm::vec3 f3 = fPt(-walkW, 0, d2), f4 = fPt(walkW, 0, d2);
            pushP(floor, f1, {0,1,0}, f1.x*0.025f, f1.z*0.025f);
            pushP(floor, f2, {0,1,0}, f2.x*0.025f, f2.z*0.025f);
            pushP(floor, f3, {0,1,0}, f3.x*0.025f, f3.z*0.025f);
            pushP(floor, f2, {0,1,0}, f2.x*0.025f, f2.z*0.025f);
            pushP(floor, f4, {0,1,0}, f4.x*0.025f, f4.z*0.025f);
            pushP(floor, f3, {0,1,0}, f3.x*0.025f, f3.z*0.025f);

            // 2 dải LED
            float ledW = 3.8f;
            glm::vec3 l1 = fPt(-walkW, 0.3f, d1), l2 = fPt(-walkW + ledW, 0.3f, d1);
            glm::vec3 l3 = fPt(-walkW, 0.3f, d2), l4 = fPt(-walkW + ledW, 0.3f, d2);
            pushFlatTri(leds, l1, l2, l3); pushFlatTri(leds, l2, l4, l3);

            glm::vec3 r1 = fPt(walkW - ledW, 0.3f, d1), r2 = fPt(walkW, 0.3f, d1);
            glm::vec3 r3 = fPt(walkW - ledW, 0.3f, d2), r4 = fPt(walkW, 0.3f, d2);
            pushFlatTri(leds, r1, r2, r3); pushFlatTri(leds, r2, r4, r3);

            // 2 bờ gờ bê tông đặc hai bên (có cả mặt đứng y=0..8 lẫn mặt trên)
            glm::vec3 cL0 = fPt(-walkW, 0, d1), cL0b = fPt(-walkW, 0, d2);
            glm::vec3 cL1 = fPt(-r, 8, d1), cL2 = fPt(-walkW, 8, d1), cL3 = fPt(-r, 8, d2), cL4 = fPt(-walkW, 8, d2);
            pushFlatTri(curbs, cL0, cL2, cL0b); pushFlatTri(curbs, cL2, cL4, cL0b);
            pushFlatTri(curbs, cL1, cL2, cL3);  pushFlatTri(curbs, cL2, cL4, cL3);

            glm::vec3 cR0 = fPt(walkW, 0, d1), cR0b = fPt(walkW, 0, d2);
            glm::vec3 cR1 = fPt(walkW, 8, d1), cR2 = fPt(r, 8, d1), cR3 = fPt(walkW, 8, d2), cR4 = fPt(r, 8, d2);
            pushFlatTri(curbs, cR0, cR0b, cR1); pushFlatTri(curbs, cR1, cR0b, cR3);
            pushFlatTri(curbs, cR1, cR2, cR3);  pushFlatTri(curbs, cR2, cR4, cR3);
        }

        // 3. Khung dầm thép 3D chịu lực uốn cong dọc hành lang
        int beamCount = std::max(2, static_cast<int>(len / 120.0f));
        for (int b = 0; b <= beamCount; ++b) {
            float d = (len * b) / beamCount;
            float rIn = r - 3.2f, rOut = r + 2.2f, hz = 2.8f;
            for (int i = 0; i < segs; ++i) {
                float t1 = 3.14159265f * (float)i / segs;
                float t2 = 3.14159265f * (float)(i + 1) / segs;
                auto bPt = [&](float rad, float th, float dOff) {
                    return origin + fwd * (d + dOff) + right * (rad * cosf(th)) + glm::vec3(0.0f, rad * sinf(th), 0.0f);
                };
                glm::vec3 i1 = bPt(rIn, t1, -hz), i2 = bPt(rIn, t2, -hz), i3 = bPt(rIn, t1, hz), i4 = bPt(rIn, t2, hz);
                glm::vec3 o1 = bPt(rOut, t1, -hz), o2 = bPt(rOut, t2, -hz), o3 = bPt(rOut, t1, hz), o4 = bPt(rOut, t2, hz);
                pushFlatTri(beams, i1, i2, i3); pushFlatTri(beams, i2, i4, i3);
                pushFlatTri(beams, o1, o3, o2); pushFlatTri(beams, o2, o3, o4);
                pushFlatTri(beams, i1, o1, i2); pushFlatTri(beams, i2, o1, o2);
                pushFlatTri(beams, i3, i4, o3); pushFlatTri(beams, i4, o4, o3);
            }
        }

        // 4. Đặt 1 Kiosk thông tin ở giữa đoạn hành lang
        if (len > 220.0f) {
            glm::vec3 kPos = origin + fwd * (len * 0.5f) + right * (walkW * 0.85f) + glm::vec3(0.0f, 12.0f, 0.0f);
            addBox(kiosks, kPos, glm::vec3(7.0f, 12.0f, 7.0f));
        }

        // 5. Vòm kính bán cầu quan sát ở đầu hoặc cuối nếu là điểm quan sát
        if (capStart) buildEndDome(origin, -fwd, right, r, glass, floor, leds);
        if (capEnd)   buildEndDome(origin + fwd * len, fwd, right, r, glass, floor, leds);
    }

    void buildEndDome(glm::vec3 center, glm::vec3 outFwd, glm::vec3 right, float r,
                      std::vector<float>& glass, std::vector<float>& floor, std::vector<float>& leds) {
        const int rings = 10, segs = 28;
        float walkW = r * 0.78f;

        for (int i = 0; i < segs; ++i) {
            float a1 = 3.14159265f * (float)i / segs;
            float a2 = 3.14159265f * (float)(i + 1) / segs;
            glm::vec3 p1 = center + right * (walkW * cosf(a1)) + outFwd * (walkW * sinf(a1));
            glm::vec3 p2 = center + right * (walkW * cosf(a2)) + outFwd * (walkW * sinf(a2));
            pushP(floor, center, {0,1,0}, center.x*0.025f, center.z*0.025f);
            pushP(floor, p1,     {0,1,0}, p1.x*0.025f, p1.z*0.025f);
            pushP(floor, p2,     {0,1,0}, p2.x*0.025f, p2.z*0.025f);

            float rIn = walkW - 3.8f;
            glm::vec3 l1 = center + right * (rIn * cosf(a1)) + outFwd * (rIn * sinf(a1)) + glm::vec3(0,0.3f,0);
            glm::vec3 l2 = center + right * (rIn * cosf(a2)) + outFwd * (rIn * sinf(a2)) + glm::vec3(0,0.3f,0);
            glm::vec3 l3 = p1 + glm::vec3(0,0.3f,0);
            glm::vec3 l4 = p2 + glm::vec3(0,0.3f,0);
            pushFlatTri(leds, l1, l3, l2); pushFlatTri(leds, l2, l3, l4);
        }

        for (int rIdx = 0; rIdx < rings; ++rIdx) {
            float lat1 = (3.14159265f * 0.5f) * (float)rIdx / rings;
            float lat2 = (3.14159265f * 0.5f) * (float)(rIdx + 1) / rings;
            for (int sIdx = 0; sIdx < segs; ++sIdx) {
                float lon1 = 3.14159265f * (float)sIdx / segs;
                float lon2 = 3.14159265f * (float)(sIdx + 1) / segs;
                auto dPt = [&](float lat, float lon) {
                    return center + right * (r * cosf(lat) * cosf(lon))
                                  + outFwd * (r * cosf(lat) * sinf(lon))
                                  + glm::vec3(0.0f, r * sinf(lat), 0.0f);
                };
                auto pushDV = [&](glm::vec3 p) {
                    pushP(glass, p, glm::normalize(p - center), 0.5f, 0.5f);
                };
                glm::vec3 p1 = dPt(lat1, lon1), p2 = dPt(lat1, lon2);
                glm::vec3 p3 = dPt(lat2, lon1), p4 = dPt(lat2, lon2);
                pushDV(p1); pushDV(p2); pushDV(p3);
                pushDV(p2); pushDV(p4); pushDV(p3);
            }
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: KHỚP NỐI NGÃ RẼ VUÔNG GÓC 90° (CORNER JUNCTION ROTUNDA)]
    // ------------------------------------------------------------------------------------------
    void buildCornerHub(float cx, float cz, float r, float openXSign, float openZSign,
                        std::vector<float>& glass, std::vector<float>& floor,
                        std::vector<float>& curbs, std::vector<float>& beams, std::vector<float>& leds) {
        const int segs = 32;
        const int rings = 8;
        glm::vec3 center(cx, 0.0f, cz);

        // Sàn tròn ngã rẽ + Vòm kính bán cầu trên đỉnh
        for (int i = 0; i < segs; ++i) {
            float a1 = 2.0f * 3.14159265f * (float)i / segs;
            float a2 = 2.0f * 3.14159265f * (float)(i + 1) / segs;
            glm::vec3 p1(cx + r * cosf(a1), 0.0f, cz + r * sinf(a1));
            glm::vec3 p2(cx + r * cosf(a2), 0.0f, cz + r * sinf(a2));
            pushP(floor, center, {0,1,0}, cx*0.025f, cz*0.025f);
            pushP(floor, p1,     {0,1,0}, p1.x*0.025f, p1.z*0.025f);
            pushP(floor, p2,     {0,1,0}, p2.x*0.025f, p2.z*0.025f);
        }

        for (int rIdx = 0; rIdx < rings; ++rIdx) {
            float lat1 = (3.14159265f * 0.5f) * (float)rIdx / rings;
            float lat2 = (3.14159265f * 0.5f) * (float)(rIdx + 1) / rings;
            for (int sIdx = 0; sIdx < segs; ++sIdx) {
                float lon1 = 2.0f * 3.14159265f * (float)sIdx / segs;
                float lon2 = 2.0f * 3.14159265f * (float)(sIdx + 1) / segs;
                float lonMid = 0.5f * (lon1 + lon2);
                float latMid = 0.5f * (lat1 + lat2);

                // Bỏ qua phần kính dưới thấp tại đúng 2 hướng thông với hành lang vuông góc
                if (latMid < 1.15f) {
                    float dirX = cosf(lonMid), dirZ = sinf(lonMid);
                    if ((dirX * openXSign > 0.45f) || (dirZ * openZSign > 0.45f)) continue;
                }

                auto dPt = [&](float lat, float lon) {
                    return glm::vec3(cx + r * cosf(lat) * cosf(lon),
                                     r * sinf(lat),
                                     cz + r * cosf(lat) * sinf(lon));
                };
                auto pushDV = [&](glm::vec3 p) {
                    pushP(glass, p, glm::normalize(p - center), 0.5f, 0.5f);
                };
                glm::vec3 p1 = dPt(lat1, lon1), p2 = dPt(lat1, lon2);
                glm::vec3 p3 = dPt(lat2, lon1), p4 = dPt(lat2, lon2);
                pushDV(p1); pushDV(p2); pushDV(p3);
                pushDV(p2); pushDV(p4); pushDV(p3);
            }
        }

        // 4 cột thép góc cua
        for (int k = 0; k < 4; ++k) {
            float ang = 0.7854f + k * 1.5708f;
            addBox(beams, glm::vec3(cx + (r - 2.0f) * cosf(ang), r * 0.35f, cz + (r - 2.0f) * sinf(ang)),
                   glm::vec3(3.5f, r * 0.35f, 3.5f));
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 6: HÀM NẠP DỮ LIỆU ĐỈNH VÀO BỘ ĐỆM VRAM GPU (VAO / VBO)]
    // ------------------------------------------------------------------------------------------
    void setupBuffer(unsigned int& vao, unsigned int& vbo, const std::vector<float>& verts) {
        if (verts.empty()) return;
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
};
